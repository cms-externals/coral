#!/bin/env python
import os, sys
from subprocess import Popen, PIPE, STDOUT
from time import sleep
import socket

#-----------------------------------------------------------------------------

sshcmd0 = "/usr/bin/ssh"
sshcmd1 = "/tmp/" + os.environ['USER'] + "/sshTunnel"

# Local host (OCI client and tunnel gateway), target host (Oracle server)
# 1. To debug, try 'sqlplus avalassi/xxx@itrac1302-v:10121/intdb11_lb.cern.ch'
# (note that intdb111/2 are defined on itrac1302/6-v, intdb11_lb on both)
# 2. For more accurate debugging of the tnsnames.ora used in this test, try
# 'sqlplus avalassi/xxx@"(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=itrac1302-v)(PORT=10121)))(CONNECT_DATA=(SERVICE_NAME=intdb11_lb.cern.ch)(INSTANCE_NAME=intdb111)(SERVER=DEDICATED)))"'
# 3. For even more debugging, try 'ssh -k -o PasswordAuthentication=no -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -N -L 45000:itrac1302-v.cern.ch:10121 $HOST &' and then 'sqlplus avalassi/xxx@"(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=localhost)(PORT=45000)))(CONNECT_DATA=(SERVICE_NAME=intdb11_lb.cern.ch)(INSTANCE_NAME=intdb111)(SERVER=DEDICATED)))"'
ghost = socket.gethostname()

# Service name
svcnm = "intdb11_lb.cern.ch" # on both nodes

# Instance name and instance host
insnm = "intdb111" # on itrac1302-v
inshs = "itrac1302"
###insnm = "intdb112" # on itrac1306-v
###inshs = "itrac1306"

# Note also that xxx-s moves connections to xxx-v, which is not good for the
# network glitch test as breaking the tunnel does not break the connection!
# Use netstat to check where the listener opens the connection socket!
###tport = inshs+"-s.cern.ch:10121" # not OK (socket opened on xxx-v)
tport = inshs+"-v.cern.ch:10121" # OK (socket opened on xxx-v)
###tport = inshs+".cern.ch:10121" # also OK (socket opened on xxx)

class NetworkTunnel:

    @staticmethod
    def tport(): return tport

    @staticmethod
    def ghost(): return ghost
    
    @staticmethod
    def svcnm(): return svcnm
    
    @staticmethod
    def insnm(): return insnm
    
    def __init__( self, lport, debug=False, kill=True ):
        self.lport = lport
        self.debug = debug
        self.sshcmd = sshcmd1 + str(lport)
        if kill: self.killall()
        os.system( "\\rm " + self.sshcmd + " > /dev/null 2>&1" )
        os.system( "\\ln -sf " + sshcmd0 + " " + self.sshcmd )

    def __del__( self ):
        self.killall() # Fix bug #73763 (avoid hang in sshTunnel bkg process)

    def dbg( self, msg ):
        if self.debug: print("NetworkTunnel (" + str(os.getpid()) + ") " + msg)

    # Kill any open sshTunnel processes for $USER
    def killall( self ):
        pids = self.ps( debug=0 )
        if ( len( pids ) == 0 ) : return
        self.dbg( "Kill all open network tunnels" )
        for pid in pids:
            print("ssh("+str(pids[0])+") Process will be killed")
            os.system( "kill -KILL " + pid )
        sleep(1)
        pids = self.ps( debug=0 )
        if ( len( pids ) != 0 ) :
            sys.stderr.write( "FATAL ERROR! Could not kill network tunnels\n" )
            for pid in pids: sys.stderr.write( "PID: " + str(pid) + "\n" )
            sys.exit(1)

    # Check if an sshTunnel process is running for $USER and return its pid
    # Return 0 if no sshTunnel process is running for $USER
    # Return -1 if more than one sshTunnel processes are running for $USER
    def ps( self, debug=1 ):
        if debug == 1: self.dbg( "Check if any network tunnels are open" )
        cmd = "ps -ef | grep " + os.environ['USER'] + " | grep " + self.sshcmd + " | grep -v grep"
        if debug == 1: self.dbg( "Execute: '" + cmd + "'" )
        try: processes = [ item[:-1] for item in Popen( cmd, shell=True, stdout=PIPE ).stdout ]
        except TypeError: processes = []
        pids = []
        if len( processes ) == 0:
            if debug == 1: self.dbg( "No network tunnel is open" )
        elif len( processes ) == 1:
            pid = processes[0].decode().split(' ',1)[1].lstrip(' ').split(' ',1)[0]
            pids.append(pid)
            if debug == 1: self.dbg( "One network tunnel is open: pid="+pid)
        else:
            for process in processes:
                pid = process.decode().split(' ',1)[1].lstrip(' ').split(' ',1)[0]
                pids.append(pid)
            if debug == 1: self.dbg( "WARNING! Several network tunnels are open: pid = "+str(pids) )
        for process in processes:
            if debug == 1 and self.debug: print(process)
        return pids

    def open(self):
        pids = self.ps( debug=0 )
        if ( len( pids ) != 0 ) :
            self.dbg( "ERROR! Cannot open a new network tunnel: one or more are already open" )
            return
        self.dbg( "Open a new network tunnel from localhost:" + str(self.lport) + " to " + tport + " through " + ghost )
        # See http://linuxcommando.blogspot.com/2008/10/how-to-disable-ssh-host-key-checking.html
        ###cmd = self.sshcmd + " -k -o PasswordAuthentication=no -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep -v Paperinik" # DEBUG bug #100763
        cmd = self.sshcmd + " -k -o PasswordAuthentication=no -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep ssh" # Output/Error to stdout
        self.dbg( "Execute: '" + cmd + "'" )
        Popen( cmd, shell=True )
        ###cmd = self.sshcmd + " -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep ssh &" # Output/Error to stdout
        ###cmd = self.sshcmd + " -N -L " + str(self.lport) + ":" + tport + " " + ghost + " -f 2>&1 | grep ssh" # Output/Error to stdout
        ###self.dbg( "Execute: '" + cmd + "'" )
        ###os.system( cmd )
        self.dbg( "Command executed" )
        self.dbg( "Sleep 1s" )
        sleep(1) # Allow time for ssh to fail (bug #100763)
        pids = self.ps( debug=0 )
        if ( len( pids ) != 1 ) :
            print("WARNING! Could not open network tunnel (yet?)")
            print("WARNING! ps returns",len(pids),"tunnels (one expected):",pids)
            print("WARNING! Sleep 3s and run ps again")
            sleep(3) # Attempt a workaround for bug #76917
            pids = self.ps( debug=0 )
        if ( len( pids ) != 1 ) :
            print("FATAL ERROR! Could not open network tunnel")
            print("FATAL ERROR! ps returns",len(pids),"tunnels (one expected):",pids)
            print("FATAL ERROR! print error on stderr and exit")
            sys.stderr.write( "FATAL ERROR! Could not open network tunnel\n" )
            sys.exit(1)
        print("ssh("+str(pids[0])+") Process started")
        self.dbg( "Sleep 1s" )
        sleep(1) # Else may get ORA-12541 on first connection attempt
        
    def close(self):
        self.dbg( "Close network tunnel" )
        pids = self.ps( debug=0 )
        if ( len( pids ) == 0 ) :
            self.dbg( "ERROR! Cannot close the network tunnel: none is open" )
            return
        elif ( len( pids )>1 ) :
            self.dbg( "WARNING! Will close more than one network tunnels" )
        self.killall()

    def glitch(self,sleepSec=1):
        self.dbg( "*** GLITCH START ***" )
        ###self.dbg( "___sleep(10)___ Query v$session before the glitch" )
        ###sleep(10)
        self.close()
        sleep(sleepSec)
        ###self.dbg( "___sleep(10)___ Query v$session during the glitch" )
        ###sleep(10)
        self.open()
        self.dbg( "*** GLITCH END   ***" )
        ###self.dbg( "___sleep(10)___ Query v$session after the glitch" )
        ###sleep(10)

#-----------------------------------------------------------------------------

if __name__ == '__main__':    
    print("__main__ Start")
    print("__main__ Instantiate NetworkTunnel manager")
    ###nt = NetworkTunnel(45000)
    nt = NetworkTunnel(45000,debug=True)
    print("__main__ Check processes")
    nt.ps()
    print("__main__ Open tunnel")
    nt.open()
    print("__main__ Check processes")
    nt.ps()
    print("__main__ Glitch tunnel")
    nt.glitch()
    print("__main__ Check processes")
    nt.ps()
    ###print "__main__ Open tunnel"
    ###nt.open() # WILL FAIL (already open)
    print("__main__ Close tunnel")
    nt.close()
    ###print "__main__ Close tunnel"
    ###nt.close() # WILL FAIL (already closed)
    print("__main__ Check processes")
    nt.ps()
    print("__main__ Delete NetworkTunnel manager")
    nt = 0
    print("__main__ Exit")
