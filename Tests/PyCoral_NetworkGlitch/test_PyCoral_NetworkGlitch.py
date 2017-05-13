#!/usr/bin/env python
import os, sys, unittest, fcntl
from NetworkTunnel import NetworkTunnel
from time import sleep, time, localtime, strftime
from signal import SIGKILL
import PyCoralTest

#============================================================================

def msghdr( ichild=0 ):
    tnow = time()
    now = strftime("%H:%M:%S",localtime(tnow))+str("%.6f"%(tnow-int(tnow)))[1:]
    if ichild == 0 :
        MSGHDR = "+++ PYTEST MAIN (%5i) @%s +++"%(os.getpid(),now)
    else:
        MSGHDR = "+++ PYTEST child(%5i) @%s +++"%(os.getpid(),now)
    return MSGHDR

#============================================================================

def createTable():
    ###print msghdr(), "Recreate the table"
    session = svc.connect( tunnelUrl, coral.access_Update )
    session.transaction().start(False)
    session.nominalSchema().dropIfExistsTable( tableName )
    description = coral.TableDescription()
    description.setName( tableName)
    description.insertColumn( 'ID', 'int' )
    description.insertColumn( 'Data', 'float' )
    description.setPrimaryKey( 'ID' )
    tableHandle = session.nominalSchema().createTable( description )
    ###print msghdr(), "Fill the table"
    bulkInserter = tableHandle.dataEditor().bulkInsert( rowBuffer, 100 )
    for i in range(100000):
        rowBuffer["ID"].setData(i)
        rowBuffer["Data"].setData(i)
        bulkInserter.processNextIteration()
    bulkInserter.flush()
    session.transaction().commit()
    sleep(1)

#============================================================================

class TestNetworkGlitch( PyCoralTest.TestCase ):

    #------------------------------------------------------------------------

    def setUp(self):
        # Call the base class method
        PyCoralTest.TestCase.setUp(self)
        # Use CORAL defaults for retrial parameters
        svc.configuration().setConnectionRetrialPeriod( retrialPeriod )
        svc.configuration().setConnectionRetrialTimeOut( retrialTimeOut )
        # Use CORAL defaults for connection sharing (enabled)
        svc.configuration().enableConnectionSharing()
        # Configure the connection service (see bug #71449)
        # - disable the CORAL connection pool cleanup
        # - connection timeout=0: "idle" connections are immediately "expired"
        svc.configuration().disablePoolAutomaticCleanUp()
        svc.configuration().setConnectionTimeOut(0)
        # Use CORAL defaults for RO transactions (serializable)
        if "CORAL_ORA_SKIP_TRANS_READONLY" in os.environ:
            del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"]
        # Disable the hack to sleep while connecting to cause ORA-24327
        if "CORAL_ORA_TEST_ORA24327_SLEEP10S" in os.environ:
            del os.environ["CORAL_ORA_TEST_ORA24327_SLEEP10S"]

    #------------------------------------------------------------------------

    def tearDown(self):
        # Purge the connection pool after each test
        svc.purgeConnectionPool()
        # Call the base class method
        PyCoralTest.TestCase.tearDown(self)
      
    #------------------------------------------------------------------------

    # Simple test that succeeds (just to test the test infrastructure!)
    def test010_createQueryAfterGlitch(self):
        dbg = True
        if dbg: print ""
        if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        ###if dbg: print msghdr(), "Session proxy is", session
        if dbg: print msghdr(), "Start a R/O transaction"
        if dbg: print msghdr(), "GLITCH.....!"
        nt.glitch() # glitch when the transaction is not yet active...
        if dbg: print msghdr(), "GLITCH OVER!"
        session.transaction().start(True)
        if dbg: print msghdr(), "Trigger reconnect (retrieve nominal schema)"
        session.nominalSchema()
        if dbg: print msghdr(), "Create a query"
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        if dbg: print msghdr(), "Retrieve the first 5 rows"
        counter = 0
        while ( counter < 5 and cursor.next() ):
            print rowBuffer[0], rowBuffer[1]
            counter += 1
        if dbg: print msghdr(), "Commit the R/O transaction"
        session.transaction().commit()

    #------------------------------------------------------------------------

    # Test for bug #73334 (presently throws an ERROR)
    def test011_ICursorPVM_bug73334(self):
        dbg = True
        if dbg: print ""
        if "LCG_NGT_SLT_NUM" in os.environ:
            print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it throws"
            return # disable in nightlies
        ###if not "ENABLE_TEST_BUG73334" in os.environ:
        ###    print "Skip test (ENABLE_TEST_BUG73334 not set)"
        ###    return # disabled (it used to crash!...)
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        ###if dbg: print msghdr(), "Session proxy is", session
        if dbg: print msghdr(), "Start a R/O transaction"
        session.transaction().start(True)
        if dbg: print msghdr(), "Create a query"
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        if dbg: print msghdr(), "Retrieve the first 5 rows"
        counter = 0
        while ( counter < 5 and cursor.next() ):
            print rowBuffer[0], rowBuffer[1]
            counter += 1
        if dbg: print msghdr(), "GLITCH.....!"
        nt.glitch()
        if dbg: print msghdr(), "GLITCH OVER!"
        if dbg: print msghdr(), "Trigger reconnect (retrieve nominal schema)"
        session.nominalSchema()
        if dbg: print msghdr(), "Retrieve the 6th row (this may fail)"
        cursor.next()
        if dbg: print msghdr(), "The 6th row was retrieved: print it"
        print rowBuffer[0], rowBuffer[1]
        counter += 1
        if dbg: print msghdr(), "Retrieve additional rows (up to 20 in total)"
        while ( counter<20 and cursor.next() ):
            print rowBuffer[0], rowBuffer[1]
            counter += 1

    #------------------------------------------------------------------------

    # Test for bug #73688 aka bug #57639 (presently succeeds)
    def test021_glitchInActiveTransRW_bug73688_akaBug57639(self):
        dbg = True
        if dbg: print ""
        if dbg: print msghdr(), "Connect R/W to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_Update )
        if dbg: print msghdr(), "Start a R/W transaction"
        session.transaction().start(False)
        if dbg: print msghdr(), "GLITCH.....!"
        nt.glitch()
        if dbg: print msghdr(), "GLITCH OVER!"
        if dbg: print msghdr(), "Try to commit... it should fail"
        try: 
            session.transaction().commit()
            self.fail( "Commit should fail for RW transaction after glitch!" )
        except coral.Exception, e:
            if dbg: print msghdr(), "Commit has failed: OK"

    #------------------------------------------------------------------------

    # Test for bug #57117 (presently FAILS with assertion)
    def test022_glitchInActiveTransSerialRO_bug57117(self):
        dbg = True
        if dbg: print ""
        if "LCG_NGT_SLT_NUM" in os.environ:
            print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it fails"
            return # disable in nightlies
        if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        if dbg: print msghdr(), "Start a R/O transaction"
        session.transaction().start(True)
        if dbg: print msghdr(), "GLITCH.....!"
        nt.glitch()
        if dbg: print msghdr(), "GLITCH OVER!"
        if dbg: print msghdr(), "Try to commit... it should fail"
        #session.transaction().commit()
        try: 
            session.transaction().commit()
            self.fail( "Commit should fail for serializable RO transaction after glitch!" )
        except coral.Exception, e:
            if dbg: print msghdr(), "Commit has failed: OK"

    #------------------------------------------------------------------------

    # Test for bug #65597 (presently succeeds... should check it used to fail!)
    def test023_glitchInActiveTransNonSerialRO_bug65597(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print ""
        if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        if dbg: print msghdr(), "Start a R/O transaction"
        session.transaction().start(True)
        if dbg: print msghdr(), "GLITCH.....!"
        nt.glitch()
        if dbg: print msghdr(), "GLITCH OVER!"
        if dbg: print msghdr(), "Try to commit... it should succeed"
        session.transaction().commit()
        if dbg: print msghdr(), "Commit has succeeded: OK"

    #------------------------------------------------------------------------

    # Test ... (presently succeeds)
    def test031_IConnectionToCursor(self):
        dbg = True
        if dbg: print ""
        self.internalTest03x_IConnectionToCursor(dbg,1)
        
    # Test ... (presently succeds)
    def test032_IConnectionToCursor(self):
        dbg = True
        if dbg: print ""
        self.internalTest03x_IConnectionToCursor(dbg,2)

    # Test ... (presently succeds)
    def test033_IConnectionToCursor(self):
        dbg = True
        if dbg: print ""
        self.internalTest03x_IConnectionToCursor(dbg,3)

    # Test ... (presently throws an ERROR)
    def test034_IConnectionToCursor(self):
        dbg = True
        if dbg: print ""
        if "LCG_NGT_SLT_NUM" in os.environ:
            print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it throws"
            return # disable in nightlies
        self.internalTest03x_IConnectionToCursor(dbg,4)

    # Test ... (presently throws an ERROR)
    def test035_IConnectionToCursor(self):
        dbg = True
        if dbg: print ""
        if "LCG_NGT_SLT_NUM" in os.environ:
            print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it throws"
            return # disable in nightlies
        self.internalTest03x_IConnectionToCursor(dbg,5)
        
    def internalTest03x_IConnectionToCursor(self, dbg, i):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print msghdr(), "Open a network tunnel"
        nt.open()
        if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        #session = svc.connect(tunnelUrl, coral.access_Update )
        if i == 1:
            if dbg: print msghdr(), "GLITCH.....!"
            nt.glitch()
            if dbg: print msghdr(), "GLITCH OVER!"
            if dbg: print msghdr(), "Glitch before transaction instantiation"
        transaction = session.transaction()
        if i == 2:
            if dbg: print msghdr(), "GLITCH.....!"
            nt.glitch()
            if dbg: print msghdr(), "GLITCH OVER!"
            if dbg: print msghdr(), "Glitch before transaction start"
        transaction.start(True)
        if i == 3:
            if dbg: print msghdr(), "GLITCH.....!"
            nt.glitch()
            if dbg: print msghdr(), "GLITCH OVER!"
            if dbg: print msghdr(), "Glitch before Nominal Schema"
        schema = session.nominalSchema()
        if i == 4:
            if dbg: print msghdr(), "GLITCH.....!"
            nt.glitch()
            if dbg: print msghdr(), "GLITCH OVER!"
            if dbg: print msghdr(), "Glitch before Query"
        query = schema.tableHandle(tableName).newQuery()
        if i == 4:
            if dbg: print msghdr(), "GLITCH.....!"
            nt.glitch()
            if dbg: print msghdr(), "GLITCH OVER!"
            if dbg: print msghdr(), "GLITCH before Query execution"
        query.defineOutput( rowBuffer )
        counter = 0
        cursor = query.execute()

        while ( cursor.next() ):
            if counter == 21: break
            print rowBuffer[0], rowBuffer[1]
            if counter == 10 and i == 5:
                if dbg: print msghdr(), "GLITCH.....!"
                nt.glitch()
                if dbg: print msghdr(), "GLITCH OVER!"
                if dbg: print msghdr(), "GLITCH during the loop on the Cursor"
            counter += 1

    def test036_IConnectionToCursorWithDoubleSession(self): # (presently throws an ERROR)
        dbg = True
        if dbg: print ""
        if "LCG_NGT_SLT_NUM" in os.environ:
            print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it throws"
            return # disable in nightlies
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print "Open a network tunnel"                    
        nt.open()
        if dbg: print "Connect R/O to:", tunnelUrl
        session1 = svc.connect(tunnelUrl, coral.access_ReadOnly )
        session2 = svc.connect(tunnelUrl, coral.access_ReadOnly )
        if dbg: print "GLITCH.....!"
        nt.glitch()
        if dbg: print "GLITCH OVER!"        
        if dbg: print "Instantiate the transaction 1"
        transaction1 = session1.transaction()
        if dbg: print "Instantiate the transaction 2"
        transaction2 = session2.transaction()
        if dbg: print "REInstantiate the transaction 1"
        session1.transaction()
        if dbg: print "GLITCH.....!"
        nt.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Start the transaction 1"
        transaction1.start(True)
        if dbg: print "GLITCH.....!"
        nt.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Start the transaction 2"
        transaction2.start(True)
        if dbg: print "ReStart the transaction 1"
        transaction1.start(True)
        if dbg: print "End"

        
        if dbg: print "GLITCH.....!"
        nt.glitch()
        if dbg: print "GLITCH OVER!"        
        if dbg: print "Use sessions again"
        if dbg: print "SCHEMA1 SESSION1"
        schema1 = session1.nominalSchema()
        if dbg: print "SCHEMA2 SESSION2"
        schema2 = session2.nominalSchema()

        if dbg: print "QUERY SCHEMA1 SESSION1"
        query1 = schema1.tableHandle(tableName).newQuery()
        if dbg: print "GLITCH.....!"
        nt.glitch()
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
        nt.glitch()
        if dbg: print "GLITCH OVER!"
        if dbg: print "Trigger a new query using the old schema"
        query2.defineOutput( rowBuffer )
        if dbg: print "Cursor"        
        cursor2 = query2.execute()
        while ( cursor2.next() ):
            print rowBuffer[0], rowBuffer[1]


    #------------------------------------------------------------------------

    # Test for ORA-24327 (presently succeeds)
    # This is bug #58522 (aka bug #65709, bug #75596)
    def test040_glitchInConnect_ora24327_bug58522(self):
        os.environ["CORAL_ORA_TEST_ORA24327_SLEEP10S"] = "1"
        svc.configuration().disableConnectionSharing() # force new connection!
        svc.configuration().setConnectionRetrialPeriod( 5 )
        newTimeOut = 15
        svc.configuration().setConnectionRetrialTimeOut( newTimeOut )
        dbg = True # needed?
        print ""
        print msghdr(), "Fork and delay dump to resync parent and child"
        # See http://stackoverflow.com/questions/871447
        rpipe, wpipe = os.pipe()
        child_pid = os.fork()
        # Parent process: connect (allow a 10s window for the glitch)
        if child_pid != 0:
            os.close(wpipe)
            rtmp, wtmp = os.pipe()
            # See http://www.parallelpython.com/component/option,com_smf/Itemid,29/topic,414.msg1225#msg1225
            fd1 = sys.__stdout__.fileno() # This fd is normally 1
            savestdout = os.dup(fd1)
            os.dup2(wtmp,fd1) # Redirect stdout to tmp pipe
            if dbg: print msghdr(), "Parent process: PID=%s" % os.getpid()
            if dbg: print msghdr(), "Connect R/O to:", tunnelUrl
            time0 = time()
            connectfailed = None
            connected = None
            try:
                session = svc.connect( tunnelUrl, coral.access_ReadOnly )
                # Internal error (e.g. sleep and/or glitch did not work)
                self.fail( "Connection succeeded? It should fail!" )
            except coral.Exception, connectfailed:
                delta = time() - time0
                print msghdr(), \
                      "Exception caught after %.3f seconds"%delta
                print msghdr(), \
                      "Sleep + Retrial timeout is %.3f seconds"%(10+newTimeOut)
                # The test succeeds if CORAL immediately throws
                if delta > 10+newTimeOut:
                    print msghdr(), \
                          "CORAL waited for the retrial timeout: FAILURE"
                else:
                    print msghdr(), \
                          "CORAL did not wait for the retrial timeout: SUCCESS"
                    connectfailed = None
            except Exception, connected:
                pass
            os.dup2(savestdout,fd1) # Restore stdout
            os.close(wtmp)
            # Cleanup (before rethrowing an exception if necessary)
            try: os.kill(child_pid,SIGKILL) # Terminate subprocess if any
            except Exception, nosubprocess: pass # No subprocess
            fcntl.fcntl(rpipe, fcntl.F_SETFL, os.O_NONBLOCK)
            rpipe = os.fdopen(rpipe,'r',0)
            fcntl.fcntl(rtmp, fcntl.F_SETFL, os.O_NONBLOCK)
            rtmp = os.fdopen(rtmp,'r',0)
            cline = None  # signal that child line must be read
            pline = None  # signal that parent line must be read
            pfirst = True # print parent line first by default
            while True:
                if cline is None:
                    try: cline = rpipe.readline()
                    except: cline = "" # This pipe has been closed
                if pline is None:
                    try: pline = rtmp.readline()
                    except: pline = "" # This pipe has been closed
                if cline == "" and pline == "":
                    break # Both pipes have been closed
                elif cline == "":
                    pfirst = True
                elif pline == "":
                    pfirst = False
                else:
                    cat = cline.find('@')
                    pat = pline.find('@')
                    if cat >=0 and pat >=0:
                        pfirst = ( pline[pat+1:pat+16] < cline[cat+1:cat+16] )
                    else:
                        pass # reuse last order (no line time, reuse last time)
                if pfirst:
                    sys.stdout.write(pline)
                    pline = None # signal that it must be read again
                else:
                    sys.stdout.write(cline)
                    cline = None # signal that it must be read again
            rpipe.close()
            rtmp.close()
            # Rethrow an exception if necessary
            print msghdr(), "Delayed dump completed"
            if connectfailed: raise connectfailed
            if connected: raise connected
        # Child subprocess: trigger a glitch after 2s and exit
        else:
            os.close(rpipe)
            sleep(1)
            wpipe = os.fdopen(wpipe,'w',0) 
            sys.stdout = wpipe # Redirect stdout to pipe
            print msghdr(1), " Child process: PID=%s" % os.getpid()
            sleep(2)
            print msghdr(1), " GLITCH.....!"
            nt.glitch()
            print msghdr(1), " GLITCH OVER!"
            print msghdr(1), " Child process: exit!"
            sys.stdout = sys.__stdout__ # Restore stdout
            wpipe.close()
            os.kill(os.getpid(),SIGKILL) # Exit

#============================================================================

if __name__ == '__main__':

    print "-"*70

    # Build the unique table name and port number
    lport = 45000
    tableName = "NETWORKGLITCHTEST"
    import PyCoralTest
    tableName = PyCoralTest.buildUniqueTableName( tableName )
    lport = PyCoralTest.buildUniquePortNumber()
    print msghdr(), "Table name:", tableName
    print msghdr(), "Local port number:", lport

    # Create the appropriate tnsnames.ora
    # [NB 'lcg_coral_nightly_proxy' is in $CORAL_AUTH_PATH/authentication.xml]
    tnsalias = "lcg_coral_nightly_proxy"
    tunnelUrl = "oracle://"+ tnsalias + "/lcg_coral_nightly"
    os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
    tnsdir = "/tmp/" + os.environ['USER']
    if "CMTCONFIG" in os.environ:
        tnsdir += "/" + os.environ["CMTCONFIG"]
    if "LCG_NGT_SLT_NAME" in os.environ:
        tnsdir += "/" + os.environ["LCG_NGT_SLT_NAME"]
    os.system( "\\mkdir -p " + tnsdir )
    tnsfile = tnsdir + "/tnsnames.ora" 
    print msghdr(), "TNS_ADMIN:", tnsdir
    os.environ['TNS_ADMIN'] = tnsdir
    tnsoutput = tnsalias+"=(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=localhost)(PORT="+str(lport)+")))(CONNECT_DATA=(SERVICE_NAME=test1.cern.ch)(INSTANCE_NAME=test11)(SERVER= DEDICATED)))"
    fd = open(tnsfile, 'w')
    fd.write(tnsoutput)
    fd.close()

    # Bootstrap CORAL
    ###os.environ['CORAL_MSGLEVEL']='Verbose'
    ###os.environ['CORAL_MSGLEVEL']='Info'
    os.environ['CORAL_MSGLEVEL']='Warning'
    import coral
    rowBuffer = coral.AttributeList()
    rowBuffer.extend("ID","int")
    rowBuffer.extend("Data","float")
    print msghdr(), "Instantiate the PyCoral connection service"
    svc = coral.ConnectionService()

    # Save CORAL default retrial parameters as global variables
    retrialPeriod=svc.configuration().connectionRetrialPeriod()
    retrialTimeOut=svc.configuration().connectionRetrialTimeOut()
    
    # Open the network tunnel
    print msghdr(), "Open a network tunnel"
    nt = NetworkTunnel(lport)
    ###nt = NetworkTunnel(lport,debug=True)
    nt.open()

    # Recreate the table?
    doCreateTable = False # default for interactive tests
    if "LCG_NGT_SLT_NUM" in os.environ:
        doCreateTable = True # default for nightly tests
    if len(sys.argv) > 1 and sys.argv[1] == "-createTable":
        doCreateTable = True # undocumented (argv[1] can also be a test)
        sys.argv.pop(1)
    if doCreateTable:
        print msghdr(), "Create the table"
        createTable()
    else:
        print msghdr(), "Do not create the table"        

    # Start the unit test (can specify one specific test as cl argument)
    unittest.main( testRunner =
                   unittest.TextTestRunner(stream=sys.stdout,verbosity=2) )
