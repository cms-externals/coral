#!/usr/bin/env python
import os, sys, unittest
from ssh_tunnel import StoneHandler
from time import sleep

#os.environ['CORAL_MSGLEVEL']='Verbose'
#os.environ['CORAL_MSGLEVEL']='Info'

os.environ['CORAL_TESTORA24327_SLEEP'] = 'TestOn' 

import coral

MSGHDR = "+++ PYCORAL TEST +++"

tableName="prova_1"

tunnelUrl = 'oracle://lcg_coral_nightly_proxy/lcg_coral_nightly'

rowBuffer = coral.AttributeList()
rowBuffer.extend("ID","int")
rowBuffer.extend("Data","float")

print MSGHDR, "Instantiate the PyCoral connection service"
svc = coral.ConnectionService()

print MSGHDR, "Instantiate a NetworkTunnel"
sh = StoneHandler()

os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['TNS_ADMIN']='.'

#============================================================================

def printSeparator():
    print "-----------------------------------------------------------------"

#============================================================================

class TestNetworkGlitch( unittest.TestCase ):

    #------------------------------------------------------------------------

    def setUp(self):
        pass

    #------------------------------------------------------------------------

    def tearDown(self):
        printSeparator()
            
    #------------------------------------------------------------------------

    def test020_ITransaction(self):
        ##os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print ""
        if dbg: print "Open a network tunnel"                    
        sh.activate()
        if dbg: print "Connect R/O to:", tunnelUrl
        session1 = svc.connect(tunnelUrl, coral.access_ReadOnly )
        session2 = svc.connect(tunnelUrl, coral.access_ReadOnly )
####### With the glitch here is is working####################        
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"        
        if dbg: print "Instantiate the transaction 1"
        transaction1 = session1.transaction()
        if dbg: print "Instantiate the transaction 2"
        transaction2 = session2.transaction()
        if dbg: print "REInstantiate the transaction 1"
        session1.transaction()
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Start the transaction 1"
        transaction1.start(True)
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Start the transaction 2"
        transaction2.start(True)
        if dbg: print "ReStart the transaction 1"
        transaction1.start(True)
        if dbg: print "End"

        
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"        
        if dbg: print "Use sessions again"
        if dbg: print "SCHEMA1 SESSION1"
        schema1 = session1.nominalSchema()
        if dbg: print "SCHEMA2 SESSION2"
        schema2 = session2.nominalSchema()

        if dbg: print "QUERY SCHEMA1 SESSION1"
        query1 = schema1.tableHandle(tableName).newQuery()
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Trigger a new query using the old schema"
        query1.defineOutput( rowBuffer )
        if dbg: print "Cursor"        
        cursor1 = query1.execute()
        while ( cursor1.next() ):
            print rowBuffer[0], rowBuffer[1]

        if dbg: print "QUERY SCHEMA2 SESSION2"
        query2 = schema2.tableHandle(tableName).newQuery()
        if dbg: print "GLITCH.....!"
        sh.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Trigger a new query using the old schema"
        query2.defineOutput( rowBuffer )
        if dbg: print "Cursor"        
        cursor2 = query2.execute()
        while ( cursor2.next() ):
            print rowBuffer[0], rowBuffer[1]

            
#============================================================================

if __name__ == '__main__':
    printSeparator()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestNetworkGlitch)
    unittest.TextTestRunner(verbosity=2).run(suite)
