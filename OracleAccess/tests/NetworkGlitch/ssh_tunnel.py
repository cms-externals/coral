import os
from os import system
from subprocess import Popen, PIPE, STDOUT
from time import sleep

def dbg():
    return "__SH (" + str(os.getpid()) + ") "

class StoneHandler:

    def __init__(self,debug = "off"):
        self.debug = debug
        self.ssh = None
        system("killall -s SIGINT ssh")

        self.a = True

    def __del__(self):
        self.deactivate()

    def activate(self):

        if self.a == False:
          return None

        if self.ssh == None:

            print dbg() + "Open tunnel from lcgaar1-v.cern.ch:10121 to localhost:45000"

            self.ssh = Popen(['/usr/bin/ssh', '-N', '-L45000:itrac507-v.cern.ch:10121', os.getenv('HOSTNAME')])

            if self.ssh == None:

              print dbg() + "FATAL: Popen doesn't return an object"

              return None

            sleep(1)

            self.ssh.poll()

            ret = self.ssh.returncode

            if ret == None:

                print dbg() + "The tunnel is active!"

            else:

               print dbg() + "The tunnel has not been activated! ErrorCode : %d" % ret

               print os.read(self.ssh.stderr,10000)

               self.ssh = None

            return self.ssh

        else:

            print dbg()+"ssh is already active"
        
    def deactivate(self):

        if self.ssh == None:
 
           print dbg()+"ssh is not active"

        else:

            print dbg() + "Closing tunnel"

            self.ssh.terminate()

            sleep(3)

            self.ssh.poll()

            if self.ssh.returncode == None:
           
              print dbg() + "Was not closed by sending the SIGTERM"

              self.ssh.kill()

              sleep(1)

            self.ssh = None

        return self.ssh

    def glitch(self):
        self.dbg( "*** GLITCH START ***" )
        self.deactivate()
        sleep(1)
        self.activate()
        self.dbg( "*** GLITCH END   ***" )

    def dbg( self, msg ):
        if self.debug: print "StoneHandler (" + str(os.getpid()) + ") " + msg

if __name__ == '__main__':    
    print "activate!"
    sh = StoneHandler("on")
    sh.activate()
    sh.activate()
    print "deactivate!"
    sh.deactivate()
    sh.deactivate()
