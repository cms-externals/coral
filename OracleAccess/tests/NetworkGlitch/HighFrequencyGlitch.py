#!/usr/bin/env python

import os, sys
from ssh_tunnel import StoneHandler
from time import sleep

class HighFrequencyGlitch:
    def __init__(self):
        self.stone = StoneHandler()
        self.stone.activate()

    def dbg(self,msg):
        return "HighFrequencyGlitch ==> "+msg
            
    def glitch(self):
        for i in range(100000000):
            self.dbg( "*** GLITCH START ***" )
            self.stone.deactivate()
            sleep(0.001)
            self.stone.activate()
            self.dbg( "*** GLITCH OVER   ***" )

    def activate(self):
        for i in range(100000000):
            self.dbg( "*** ACTIVATE ***" )
            self.stone.activate()
            sleep(0.1)

    def deactivate(self):
        for i in range(100000000):
            self.dbg( "*** DEACTIVATE ***" )
            self.stone.deactivate()
            sleep(0.1)
