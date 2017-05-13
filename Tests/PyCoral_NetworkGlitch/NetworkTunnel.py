#!/bin/env python
import os, sys
from subprocess import Popen, PIPE, STDOUT
from time import sleep
import socket

#-----------------------------------------------------------------------------

sshcmd0 = "/usr/bin/ssh"
sshcmd1 = "/tmp/" + os.environ['USER'] + "/sshTunnel"

# Local host (OCI client and tunnel gateway), target host (Oracle server)
ghost = socket.gethostname()
tport = "itrac507-v.cern.ch:10121"

class NetworkTunnel:

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
        if self.debug: print "NetworkTunnel (" + str(os.getpid()) + ") " + msg

    # Kill any open sshTunnel processes for $USER
    def killall( self ):
        pids = self.ps( debug=0 )
        if ( len( pids ) == 0 ) : return
        self.dbg( "Kill all open network tunnels" )
        for pid in pids:
            print "ssh("+str(pids[0])+") Process will be killed"
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
        processes = [ item[:-1] for item in Popen( cmd, shell=True, stdout=PIPE ).stdout ]
        pids = []
        if len( processes ) == 0:
            if debug == 1: self.dbg( "No network tunnel is open" )
        elif len( processes ) == 1:
            pid = processes[0].split(' ',1)[1].lstrip(' ').split(' ',1)[0]
            pids.append(pid)
            if debug == 1: self.dbg( "One network tunnel is open: pid="+pid)
        else:
            for process in processes:
                pid = process.split(' ',1)[1].lstrip(' ').split(' ',1)[0]
                pids.append(pid)
            if debug == 1: self.dbg( "WARNING! Several network tunnels are open: pid = "+str(pids) )
        for process in processes:
            if debug == 1 and self.debug: print process
        return pids

    def open(self):
        pids = self.ps( debug=0 )
        if ( len( pids ) != 0 ) :
            self.dbg( "ERROR! Cannot open a new network tunnel: one or more are already open" )
            return
        self.dbg( "Open a new network tunnel from localhost:" + str(self.lport) + " to " + tport + " through " + ghost )
        # See http://linuxcommando.blogspot.com/2008/10/how-to-disable-ssh-host-key-checking.html
        cmd = self.sshcmd + " -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep ssh" # Output/Error to stdout
        ###cmd = self.sshcmd + " -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep ssh" # Output/Error to stdout
        self.dbg( "Execute: '" + cmd + "'" )
        Popen( cmd, shell=True )
        ###cmd = self.sshcmd + " -N -L " + str(self.lport) + ":" + tport + " " + ghost + " 2>&1 | grep ssh &" # Output/Error to stdout
        ###cmd = self.sshcmd + " -N -L " + str(self.lport) + ":" + tport + " " + ghost + " -f 2>&1 | grep ssh" # Output/Error to stdout
        ###self.dbg( "Execute: '" + cmd + "'" )
        ###os.system( cmd )
        self.dbg( "Command executed" )
        pids = self.ps( debug=0 )
        if ( len( pids ) != 1 ) :
            print "WARNING! Could not open network tunnel (yet?)"
            print "WARNING! ps returns",len(pids),"tunnels (one expected):"
            print pids
            print "WARNING! Sleep 3s and run ps again"
            sleep(3) # Attempt a workaround for bug #76917
            pids = self.ps( debug=0 )
        if ( len( pids ) != 1 ) :
            print "FATAL ERROR! Could not open network tunnel"
            print "FATAL ERROR! ps returns",len(pids),"tunnels (one expected):"
            print pids
            print "FATAL ERROR! print error on stderr and exit"
            sys.stderr.write( "FATAL ERROR! Could not open network tunnel\n" )
            sys.exit(1)
        print "ssh("+str(pids[0])+") Process started"
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

    def glitch(self):
        self.dbg( "*** GLITCH START ***" )
        self.close()
        sleep(1)
        self.open()
        self.dbg( "*** GLITCH END   ***" )

#-----------------------------------------------------------------------------

if __name__ == '__main__':    
    print "__main__ Start"
    print "__main__ Instantiate NetworkTunnel manager"
    ###nt = NetworkTunnel(45000)
    nt = NetworkTunnel(45000,debug=True)
    print "__main__ Check processes"
    nt.ps()
    print "__main__ Open tunnel"
    nt.open()
    print "__main__ Glitch tunnel"
    nt.glitch()
    print "__main__ Check processes"
    nt.ps()
    ###print "__main__ Open tunnel"
    ###nt.open() # WILL FAIL (already open)
    print "__main__ Close tunnel"
    nt.close()
    ###print "__main__ Close tunnel"
    ###nt.close() # WILL FAIL (already closed)
    print "__main__ Delete NetworkTunnel manager"
    nt = 0
    print "__main__ Exit"
