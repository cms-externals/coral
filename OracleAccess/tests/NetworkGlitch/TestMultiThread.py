#!/usr/bin/env python

import os
import re
import time
import sys

from threading import Thread
from PyCoralTest_for_NetworkInterruption import NetworkInterruption
from HighFrequencyGlitch import HighFrequencyGlitch
from ssh_tunnel import StoneHandler

class testit(Thread):
   def __init__ (self,Stone, th=0):
      print "_Thread n.", th
      Thread.__init__(self)
      self.sh = Stone
      self.th = th

   def run(self):
      self.ni = NetworkInterruption(self.sh,self.th)
      #    self.ni.createTable("prova_1")
      self.ni.query("prova_1")

class testitGlitch(Thread):
   def __init__ (self):
      Thread.__init__(self)

   def run(self):
      HighFrequencyGlitch().glitch()
      
class testitActivate(Thread):
   def __init__ (self):
      Thread.__init__(self)

   def run(self):
      HighFrequencyGlitch().activate()

class testitDeactivate(Thread):
   def __init__ (self):
      Thread.__init__(self)

   def run(self):
      HighFrequencyGlitch().deactivate()

      

threadlist = []
sh = StoneHandler()
sh.activate()

glitch = testitGlitch()
threadlist.append(glitch)
glitch.start()
time.sleep(10)

##activate = testitActivate()
##deactivate = testitActivate()
##threadlist.append(activate)
##threadlist.append(deactivate)
##activate.start()
##deactivate.start()


for th in range(1):
   test = testit(sh,th)
   threadlist.append(test)
   test.start()

for th in threadlist:
   th.join()

print "DONE!!!"
