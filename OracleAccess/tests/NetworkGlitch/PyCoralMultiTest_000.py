#!/usr/bin/env python
import os, sys, unittest
from ssh_tunnel import StoneHandler
from time import sleep
from time import time

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

        out_file = open("test_000.txt","w")
        
        ##os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print ""
        if dbg: print "Open a network tunnel"                    
        sh.activate()
        if dbg: print "Connect R/O to:", tunnelUrl

        start = time()
        session = svc.connect(tunnelUrl, coral.access_Update )
        step = time() - start
        print "Session time 1: ", step
        out_file.write("Session time 1: "+ str(step)+"\n")
        
        if dbg: print "Instantiate the transaction"

        start = time()
        transaction = session.transaction()
        step = time() - start
        print "Session time 2: ", step
        out_file.write("Session time 2: "+ str(step)+"\n")
                
        if dbg: print "Transaction start"
        
        start = time()
        transaction.start()
        step = time() - start
        print "Session time 3: ", step
        out_file.write("Session time 3: "+ str(step)+"\n")

        if dbg: print "Schema"

        start = time()
        schema = session.nominalSchema()
        step = time() - start
        print "Session time 4: ", step
        out_file.write("Session time 4: "+ str(step)+"\n")
        
        if dbg: print "Query"

        start = time()
        query = schema.tableHandle(tableName).newQuery()
        step = time() - start
        print "Session time 5: ", step
        out_file.write("Session time 5: "+ str(step)+"\n")
                
        if dbg: print "Trigger a new query using the old schema"
        query.defineOutput( rowBuffer )
        if dbg: print "Cursor"

        start = time()
        cursor = query.execute()
        step = time() - start
        print "Session time 6: ", step
        out_file.write("Session time 6: "+ str(step)+"\n")
                
        if dbg: print "Before loop"
        while ( cursor.next() ):
            print rowBuffer[0], rowBuffer[1]

        out_file.close()
#============================================================================

if __name__ == '__main__':
    printSeparator()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestNetworkGlitch)
    unittest.TextTestRunner(verbosity=2).run(suite)
