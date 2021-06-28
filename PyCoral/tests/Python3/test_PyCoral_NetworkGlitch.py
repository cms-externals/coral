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

def createTable( theTableName, doRecreateTable ):
    ###print msghdr(), "Create the table if needed"
    session = svc.connect( tunnelUrl, coral.access_Update )
    print(msghdr(), "Successfully connected to", tunnelUrl) # after a few ORA-12541 sometimes...
    session.transaction().start(False)
    if doRecreateTable:
        print(msghdr(), "Drop and recreate the table")
        session.nominalSchema().dropIfExistsTable( theTableName )
    elif session.nominalSchema().existsTable( theTableName ):
        ###print msghdr(), "Table exists and need not be recreated"
        session.transaction().commit() # minor fix...
        return
    else:
        print(msghdr(), "Table does not exist and will be created") # bug #92376
    description = coral.TableDescription()
    description.setName( theTableName )
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

    def vFail(self,message): # verbose fail
        print("FAIL!",message)
        self.fail( message )

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
        # Disable the hack to ignore the connection probe to cause ORA-24327
        if "CORAL_ORA_TEST_ORA24327_KEEPSTALECONN" in os.environ:
            del os.environ["CORAL_ORA_TEST_ORA24327_KEEPSTALECONN"]

    #------------------------------------------------------------------------

    def tearDown(self):
        # Purge the connection pool after each test
        svc.purgeConnectionPool()
        # Call the base class method
        PyCoralTest.TestCase.tearDown(self)

    #------------------------------------------------------------------------

    # "Simple" test of infrastructure (presently succeeds, it does reconnect)
    # [OK: should reconnect because transaction is inactive]
    def test010_createQueryAfterGlitch(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        ###if dbg: print msghdr(), "Session proxy is", session
        if dbg: print(msghdr(), "Start a R/O transaction")
        if dbg: print(msghdr(), "GLITCH.....!")
        nt.glitch() # glitch when the transaction is not yet active...
        if dbg: print(msghdr(), "GLITCH OVER!")
        try:
            session.transaction().start(True)
            if dbg: print(msghdr(), "Trigger reconnect (fetch nominal schema)")
            session.nominalSchema()
            if dbg: print(msghdr(), "Create a query")
            query = session.nominalSchema().tableHandle(tableName).newQuery()
            query.addToOrderList( "ID ASC" )
            query.defineOutput( rowBuffer )
            query.setMemoryCacheSize(0)
            query.setRowCacheSize(1)
            cursor = query.execute()
            if dbg: print(msghdr(), "Retrieve the first 5 rows")
            counter = 0
            while ( counter < 5 and cursor.next() ):
                print(rowBuffer[0], rowBuffer[1])
                counter += 1
            if dbg: print(msghdr(), "Commit the R/O transaction")
            session.transaction().commit()
        except Exception as e:
            print("ERROR!",e)
            session = 0 # Clean up before leaving the test
            raise e

    #------------------------------------------------------------------------

    # Test bug #73334 (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test011_ICursorPVM_bug73334(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        ###if not "ENABLE_TEST_BUG73334" in os.environ:
        ###    print "Skip test (ENABLE_TEST_BUG73334 not set)"
        ###    return # disabled (it used to crash!...)
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        ###if dbg: print msghdr(), "Session proxy is", session
        if dbg: print(msghdr(), "Start a R/O transaction")
        session.transaction().start(True)
        if dbg: print(msghdr(), "Create a query")
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        if dbg: print(msghdr(), "Retrieve the first 5 rows")
        counter = 0
        while ( counter < 5 and cursor.next() ):
            print(rowBuffer[0], rowBuffer[1])
            counter += 1
        if dbg: print(msghdr(), "GLITCH.....!")
        nt.glitch()
        if dbg: print(msghdr(), "GLITCH OVER!")
        if dbg: print(msghdr(), "Trigger reconnect (retrieve nominal schema)")
        session.nominalSchema()
        ###self.dbg( "___sleep(10)___ Query v$session after reconnecting" )
        ###sleep(10)
        if dbg: print(msghdr(), "Retrieve the 6th row (this should fail)")
        try:
            cursor.next()
            self.vFail( "cursor.next() should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            ###exp = "Session is no longer valid"
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )
        self.assertEqual( session.transaction().isReadOnly(), True )
        #if dbg: print msghdr(), "The 6th row was retrieved(?): print it"
        #print rowBuffer[0], rowBuffer[1]
        #counter += 1
        #if dbg: print msghdr(), "Retrieve additional rows (up to 20 in total)"
        #while ( counter<20 and cursor.next() ):
        #    print rowBuffer[0], rowBuffer[1]
        #    counter += 1

    #------------------------------------------------------------------------

    # Test bug #73688 aka #57639 (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test021_glitchInActiveTransRW_bug73688_akaBug57639(self):
        dbg = True
        if dbg: print("")
        if dbg: print(msghdr(), "Connect R/W to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_Update )
        if dbg: print(msghdr(), "Start a R/W transaction")
        session.transaction().start(False)
        if dbg: print(msghdr(), "GLITCH.....!")
        nt.glitch()
        if dbg: print(msghdr(), "GLITCH OVER!")
        if dbg: print(msghdr(), "Try to commit... it should fail")
        try:
            session.transaction().commit()
            self.vFail( "Commit should fail for RW transaction after glitch!" )
        except coral.Exception as error:
            if dbg: print(msghdr(), "Commit has failed: OK")
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    #------------------------------------------------------------------------

    # Test bug #57117 (presently fails: CORAL does not reconnect)
    # [OK: should NOT reconnect because serializable RO tx has already started]
    def test022_glitchInActiveTransSerialRO_bug57117(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        if dbg: print(msghdr(), "Start a R/O transaction")
        session.transaction().start(True)
        if dbg: print(msghdr(), "GLITCH.....!")
        nt.glitch()
        if dbg: print(msghdr(), "GLITCH OVER!")
        if dbg: print(msghdr(), "Try to commit... it should fail")
        #session.transaction().commit()
        try:
            session.transaction().commit()
            self.vFail( "Commit should fail for serializable RO transaction after glitch!" )
        except coral.Exception as error:
            if dbg: print(msghdr(), "Commit has failed: OK")
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                ###session = 0 # Clean up before leaving the test
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    #------------------------------------------------------------------------

    # Test bug #65597 (presently succeeds: CORAL commit is a no-op)
    # [OK: can commit active SKIP/RO tx after glitch, bug #65597 is fixed]
    def test023_glitchInActiveTransNonSerialRO_bug65597(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        if dbg: print(msghdr(), "Start a R/O transaction")
        session.transaction().start(True)
        if dbg: print(msghdr(), "GLITCH.....!")
        nt.glitch()
        if dbg: print(msghdr(), "GLITCH OVER!")
        if dbg: print(msghdr(), "Try to commit... it should succeed")
        session.transaction().commit()
        if dbg: print(msghdr(), "Commit has succeeded: OK")
        self.assertEqual( session.transaction().isReadOnly(), True )

    #------------------------------------------------------------------------

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because SKIP/RO tx has not started yet]
    def test031a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,1)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because serializable RO tx has not started yet]
    def test031b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,1)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because RW tx has not started yet]
    def test031c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant!
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,1,False)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because SKIP/RO tx has not started yet]
    def test032a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,2)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because serializable RO tx has not started yet]
    def test032b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,2)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because RW tx has not started yet]
    def test032c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant!
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,2,False)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because tx has already started but is SKIP/RO]
    def test033a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,3)

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because serializable RO tx has already started]
    def test033b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,3)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test033c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,3,False)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because tx has already started but is SKIP/RO]
    def test034a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,4)

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because serializable RO tx has already started]
    def test034b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,4)
            self.vFail( "Test #4 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test034c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,4,False)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because tx has already started but is SKIP/RO]
    def test035a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        self.internalTest03x_IConnectionToCursor(dbg,5)

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because serializable RO tx has already started]
    def test035b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,5)
            self.vFail( "Test #5 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test035c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,5,False)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test036a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,6)
            self.vFail( "Test #6 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test036b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,6)
            self.vFail( "Test #6 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test036c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,6,False)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test037a_IConnectionToCursor(self):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,7)
            self.vFail( "Test #6 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test037b_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # in setUp
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,7)
            self.vFail( "Test #6 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeeds: CORAL does not reconnect)
    # NB: This does fail with ORA-03113 (OCICursor uses a stale OCISvcCtx!)
    # [OK: should NOT reconnect because cursor loop has already started]
    def test037c_IConnectionToCursor(self):
        ###del os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] # irrelevant
        dbg = True
        if dbg: print("")
        try:
            self.internalTest03x_IConnectionToCursor(dbg,7,False)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Catch exceptions in test 031 to 037
    def internalTest03x_IConnectionToCursor(self, dbg, i, txRO=True):
        try:
            self.internalTest03x_IConnectionToCursor_wrapped( dbg, i, txRO )
        except Exception as e:
            print("ERROR!",e)
            raise e

    # Main driver for tests 031 to 037
    def internalTest03x_IConnectionToCursor_wrapped(self, dbg, i, txRO=True):
        if dbg: print(msghdr(), "Open a network tunnel")
        nt.open()
        if txRO:
            if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
            session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        else:
            if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
            session = svc.connect( tunnelUrl, coral.access_Update )
        if i == 1:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before transaction instantiation")
        transaction = session.transaction()
        if i == 2:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before transaction start")
        transaction.start(txRO)
        if i == 3:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before Nominal Schema")
        schema = session.nominalSchema()
        if i == 4:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before Query")
        query = schema.tableHandle(tableName).newQuery()
        if i == 5:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "GLITCH before Query execution")
        query.defineOutput( rowBuffer )
        counter = 0
        cursor = query.execute()
        if i == 6:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "GLITCH before the loop on the cursor")
        while ( cursor.next() ):
            if counter == 21: break
            print(rowBuffer[0], rowBuffer[1])
            if counter == 10 and i == 7:
                if dbg: print(msghdr(), "GLITCH.....!")
                nt.glitch()
                if dbg: print(msghdr(), "GLITCH OVER!")
                if dbg: print(msghdr(), "GLITCH during the loop on the Cursor")
            counter += 1

    #------------------------------------------------------------------------

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because RW tx has not started yet]
    def test041_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        self.internalTest04x_IConnectionToUpdate(dbg,1)

    # Test ... (presently succeeds: CORAL does reconnect)
    # [OK: should reconnect because RW tx has not started yet]
    def test042_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        self.internalTest04x_IConnectionToUpdate(dbg,2)

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test043_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,3)
            self.vFail( "Test #3 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test044_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,4)
            self.vFail( "Test #4 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test045_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,5)
            self.vFail( "Test #5 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test046_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,6)
            self.vFail( "Test #6 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test047_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,7)
            self.vFail( "Test #7 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL does not reconnect)
    # [OK: should NOT reconnect because RW tx has already started]
    def test048_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest04x_IConnectionToUpdate(dbg,8)
            self.vFail( "Test #8 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )

    # Test ... (presently succeds, CORAL no-op does not need to reconnect)
    # [OK: should reconnect because RW tx has already been committed]
    def test049_IConnectionToUpdate(self):
        dbg = True
        if dbg: print("")
        self.internalTest04x_IConnectionToUpdate(dbg,9)

    # Catch exceptions in test 041 to 049
    def internalTest04x_IConnectionToUpdate(self, dbg, i):
        try:
            self.internalTest04x_IConnectionToUpdate_wrapped( dbg, i )
        except Exception as e:
            print("ERROR!",e)
            raise e

    # Main driver for tests 041 to 049
    def internalTest04x_IConnectionToUpdate_wrapped(self, dbg, i):
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print(msghdr(), "Open a network tunnel")
        nt.open()
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect(tunnelUrl, coral.access_Update )
        if i == 1:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before transaction instantiation")
        transaction = session.transaction()
        if i == 2:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before transaction start")
        transaction.start(False)
        if i == 3:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before Nominal Schema")
        schema = session.nominalSchema()
        if dbg: print("Check if table exists and in case drops it")
        schema.dropIfExistsTable( tableNameUpdate )
        if dbg: print("Create the table")
        description = coral.TableDescription()
        description.setName( tableNameUpdate )
        description.insertColumn( 'ID', 'int' )
        description.insertColumn( 'Data', 'float' )
        if i == 4:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before TableHandle")
        tableHandle = schema.createTable( description )
        if i == 5:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "GLITCH before dataEditor")
        editor = tableHandle.dataEditor()
        ## BULK INSERT
        if i == 6:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "GLITCH before the bulk insert")
        bulkInserter = editor.bulkInsert( rowBuffer, 10 )
        for counter in range(10):
            if counter == 5 and i == 7:
                if dbg: print(msghdr(), "GLITCH.....!")
                nt.glitch()
                if dbg: print(msghdr(), "GLITCH OVER!")
                if dbg: print(msghdr(), "GLITCH during the loop of the data bulk insert")
            rowBuffer["ID"].setData(counter+1)
            rowBuffer["Data"].setData(counter+1)
            bulkInserter.processNextIteration()
        bulkInserter.flush()
        ## NORMAL INSERT
        for counter in range(10,20):
            if counter == 15 and i == 8:
                if dbg: print(msghdr(), "GLITCH.....!")
                nt.glitch()
                if dbg: print(msghdr(), "GLITCH OVER!")
                if dbg: print(msghdr(), "GLITCH during the loop of the data normal insert")
            rowBuffer["ID"].setData(counter+1)
            rowBuffer["Data"].setData(counter+1)
        transaction.commit()
        if i == 9:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "GLITCH after commit")
        transaction = session.transaction()

    #------------------------------------------------------------------------

    # Test two non-serializ. R/O sessions and one R/W, with connection sharing
    def test136a_IConnectionToCursorWithThreeSessions_connSharing(self):
        coral.MessageStream("").setMsgVerbosity(coral.message_Level_Info)
        svc.configuration().enableConnectionSharing() # use same connection!
        error = None
        try: self.internalTest136_IConnectionToCursorWithThreeSessions()
        except Exception as error: print("ERROR!",error)
        coral.MessageStream("").setMsgVerbosity(msgLevel)
        if error: raise error

    # Test two non-serializ. R/O sessions and one R/W, without connection sharing
    def test136b_IConnectionToCursorWithThreeSessions_noConnSharing(self):
        coral.MessageStream("").setMsgVerbosity(coral.message_Level_Info)
        svc.configuration().disableConnectionSharing() # force new connection!
        error = None
        try: self.internalTest136_IConnectionToCursorWithThreeSessions()
        except Exception as error: print("ERROR!",error)
        coral.MessageStream("").setMsgVerbosity(msgLevel)
        if error: raise error

    # Test three sessions (presently succeeds: CORAL does reconnects two R/O)
    # [OK: should reconnect, cursor loop has not started yet in SKIP/RO tx]
    def internalTest136_IConnectionToCursorWithThreeSessions(self):
        dbg = True
        if dbg: print("")
        os.environ["CORAL_ORA_SKIP_TRANS_READONLY"] = "1"
        if dbg: print("Open a network tunnel")
        nt.open()
        if dbg: print("Connect R/O to:", tunnelUrl)
        session1 = svc.connect(tunnelUrl, coral.access_ReadOnly )
        session2 = svc.connect(tunnelUrl, coral.access_ReadOnly )
        if dbg: print("Connect R/W to:", tunnelUrl)
        session3 = svc.connect(tunnelUrl, coral.access_Update )
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Instantiate the transaction 1")
        transaction1 = session1.transaction()
        if dbg: print("Instantiate the transaction 2")
        transaction2 = session2.transaction()
        if dbg: print("Instantiate the transaction 3")
        transaction3 = session3.transaction()
        if dbg: print("ReInstantiate the transaction 1")
        session1.transaction()
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Start the transaction 1")
        transaction1.start(True)
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Start the transaction 2")
        transaction2.start(True)
        if dbg: print("ReStart the transaction 1")
        transaction1.start(True) # Will print a warning but will not fail
        if dbg: print("Start the RW transaction 3")
        transaction3.start(False)
        if dbg: print("End")
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Use sessions again")
        if dbg: print("SCHEMA1 SESSION1")
        schema1 = session1.nominalSchema()
        self.assertEqual( session1.transaction().isReadOnly(), True )
        if dbg: print("SCHEMA2 SESSION2")
        schema2 = session2.nominalSchema()
        self.assertEqual( session2.transaction().isReadOnly(), True )
        if dbg: print("QUERY SCHEMA1 SESSION1")
        query1 = schema1.tableHandle(tableName).newQuery()
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Commit R/W transaction 3 - should fail")
        try:
            transaction3.commit()
            self.vFail( "transaction3.commit() should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )
            if dbg: print("Release R/W transaction 3")
            transaction3 = 0 # Force disconnect immediately
            if dbg: print("Release R/W session 3")
            session3 = 0 # Force disconnect immediately
            if dbg: print("Succesfully released R/W session 3")
        if dbg: print("Trigger a new query using the old schema")
        query1.defineOutput( rowBuffer )
        if dbg: print("Cursor")
        cursor1 = query1.execute() # Throws in CORAL_2_3_20 (should succeed)
        i = 0
        while ( cursor1.next() ):
            if i == 10: break
            print(rowBuffer[0], rowBuffer[1])
            i += 1
        if dbg: print("QUERY SCHEMA2 SESSION2")
        query2 = schema2.tableHandle(tableName).newQuery()
        if dbg: print("GLITCH.....!")
        nt.glitch()
        if dbg: print("GLITCH OVER!")
        if dbg: print("Trigger a new query in session2 using the old schema")
        query2.defineOutput( rowBuffer )
        if dbg: print("Cursor")
        cursor2 = query2.execute()
        i = 0
        while ( cursor2.next() ):
            if i == 10: break
            print(rowBuffer[0], rowBuffer[1])
            i += 1
        if dbg: print("Create a new query in session1 using the old schema")
        query1 = schema1.tableHandle(tableName).newQuery()
        query1.defineOutput( rowBuffer )
        if dbg: print("Cursor")
        cursor1 = query1.execute()
        i = 0
        while ( cursor1.next() ):
            if i == 10: break
            print(rowBuffer[0], rowBuffer[1])
            i += 1
        if dbg: print("Create a new query in session2 using the old schema")
        query2 = schema1.tableHandle(tableName).newQuery()
        query2.defineOutput( rowBuffer )
        if dbg: print("Cursor")
        cursor2 = query2.execute()
        i = 0
        while ( cursor2.next() ):
            if i == 10: break
            print(rowBuffer[0], rowBuffer[1])
            i += 1

    #------------------------------------------------------------------------

    ### Test ORA-24327 (presently succeeds: no waiting for retrial timeout)
    ### [OK: bug #58522 (aka bug #65709, bug #75596) has been fixed]
    ### This was also affected by bug #94103 and showing bug #94114.
    def test140_glitchInConnect_ora24327_bug58522(self):
        #if "LCG_NGT_SLT_NUM" in os.environ: # was failing (bug #94103)
        #    print "Skip test in nightlies (LCG_NGT_SLT_NUM is set) - it fails"
        #    return # disable in nightlies
        os.environ["CORAL_ORA_TEST_ORA24327_SLEEP10S"] = "1"
        os.environ["CORAL_ORA_TEST_ORA24327_KEEPSTALECONN"] = "1" # bug 94103
        svc.configuration().disableConnectionSharing() # force new connection!
        svc.configuration().setConnectionRetrialPeriod( 5 )
        newTimeOut = 15
        svc.configuration().setConnectionRetrialTimeOut( newTimeOut )
        dbg = True # needed?
        print("")
        print(msghdr(), "Fork and delay dump to resync parent and child")
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
            if dbg: print(msghdr(), "Parent process: PID=%s" % os.getpid())
            time0 = time()
            connectfailed = None
            connected = None
            try:
                if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
                session = svc.connect( tunnelUrl, coral.access_ReadOnly )
                session.transaction().start(True) # test bug #94103 and #94114
                session.transaction().commit()    # test bug #94103 and #94114
                # Internal error (e.g. sleep and/or glitch did not work)
                self.vFail( "Connection succeeded? It should fail!" )
            except coral.Exception as stuff:
                delta = time() - time0
                print(msghdr(), \
                      "Exception caught:", connectfailed)
                print(msghdr(), \
                      "Exception caught after %.3f seconds"%delta)
                print(msghdr(), \
                      "Sleep + Retrial timeout is %.3f seconds"%(10+newTimeOut))
                # The test succeeds if CORAL immediately throws
                if delta > 10+newTimeOut:
                    print(msghdr(), \
                          "CORAL waited for the retrial timeout: FAILURE")
                    connectfailed = stuff
                else:
                    print(msghdr(), \
                          "CORAL did not wait for the retrial timeout: SUCCESS")
                    connectfailed = None
            except Exception as connected:
                pass
            os.dup2(savestdout,fd1) # Restore stdout
            os.close(wtmp)
            # Cleanup (before rethrowing an exception if necessary)
            try: os.kill(child_pid,SIGKILL) # Terminate subprocess if any
            except Exception as nosubprocess: pass # No subprocess
            fcntl.fcntl(rpipe, fcntl.F_SETFL, os.O_NONBLOCK)
            rpipe = os.fdopen(rpipe,'rb',0)
            fcntl.fcntl(rtmp, fcntl.F_SETFL, os.O_NONBLOCK)
            rtmp = os.fdopen(rtmp,'rb',0)
            cline = None  # signal that child line must be read
            pline = None  # signal that parent line must be read
            pfirst = True # print parent line first by default
            while True:
                if cline is None:
                    try: cline = rpipe.readline().decode()
                    except: cline = "" # This pipe has been closed
                if pline is None:
                    try: pline = rtmp.readline().decode()
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
            print(msghdr(), "Delayed dump completed")
            if connectfailed: raise connectfailed
            if connected: raise connected
        # Child subprocess: trigger a glitch after 2s and exit
        else:
            os.close(rpipe)
            sleep(1)
            #wpipe = os.fdopen(wpipe,'wb',0)
            #sys.stdout = wpipe # Redirect stdout to pipe
            print(msghdr(1), " Child process: PID=%s" % os.getpid())
            sleep(2)
            print(msghdr(1), " GLITCH.....!")
            nt.glitch()
            print(msghdr(1), " GLITCH OVER!")
            print(msghdr(1), " Child process: exit!")
            #sys.stdout = sys.__stdout__ # Restore stdout
            #wpipe.close()
            os.kill(os.getpid(),SIGKILL) # Exit

    #------------------------------------------------------------------------

    # Test connection retrial during reconnections
    # [OK: should be able to reconnect eventually]
    def test150_reconnectWithTimeout_bug94492(self):
        dbg = True
        print("")
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect( tunnelUrl, coral.access_ReadOnly )
        print(msghdr(), "Fork and delay dump to resync parent and child")
        # See http://stackoverflow.com/questions/871447
        rpipe, wpipe = os.pipe()
        child_pid = os.fork()
        # Parent process: start a transaction after 2s (during the 5s glitch)
        if child_pid != 0:
            os.close(wpipe)
            rtmp, wtmp = os.pipe()
            # See http://www.parallelpython.com/component/option,com_smf/Itemid,29/topic,414.msg1225#msg1225
            fd1 = sys.__stdout__.fileno() # This fd is normally 1
            savestdout = os.dup(fd1)
            os.dup2(wtmp,fd1) # Redirect stdout to tmp pipe
            if dbg: print(msghdr(), "Parent process: PID=%s" % os.getpid())
            time0 = time()
            failed = None
            try:
                if dbg: print(msghdr(), "Sleep 2s and start a R/O transaction")
                sleep(2)
                session.transaction().start(True)
                if dbg: print(msghdr(), "OK transaction was started")
                session.transaction().commit()
            except Exception as failed:
                delta = time() - time0
                print(msghdr(), "Exception caught:", failed)
                print(msghdr(), "Exception caught after %.3f seconds"%delta)
            os.dup2(savestdout,fd1) # Restore stdout
            os.close(wtmp)
            # Cleanup (before rethrowing an exception if necessary)
            try: os.kill(child_pid,SIGKILL) # Terminate subprocess if any
            except Exception as nosubprocess: pass # No subprocess
            fcntl.fcntl(rpipe, fcntl.F_SETFL, os.O_NONBLOCK)
            rpipe = os.fdopen(rpipe,'rb',0)
            fcntl.fcntl(rtmp, fcntl.F_SETFL, os.O_NONBLOCK)
            rtmp = os.fdopen(rtmp,'rb',0)
            cline = None  # signal that child line must be read
            pline = None  # signal that parent line must be read
            pfirst = True # print parent line first by default
            while True:
                if cline is None:
                    try: cline = rpipe.readline().decode()
                    except: cline = "" # This pipe has been closed
                if pline is None:
                    try: pline = rtmp.readline().decode()
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
            print(msghdr(), "Delayed dump completed")
            if failed: raise failed
        # Child subprocess: trigger a 5s glitch and exit
        else:
            os.close(rpipe)
            sleep(1)
            #wpipe = os.fdopen(wpipe,'w',0)
            #sys.stdout = wpipe # Redirect stdout to pipe
            print(msghdr(1), " Child process: PID=%s" % os.getpid())
            print(msghdr(1), " GLITCH (for 5 seconds).....!")
            nt.glitch(5)
            print(msghdr(1), " GLITCH OVER!")
            print(msghdr(1), " Child process: exit!")
            #sys.stdout = sys.__stdout__ # Restore stdout
            #wpipe.close()
            os.kill(os.getpid(),SIGKILL) # Exit

    #------------------------------------------------------------------------

    # Test glitch before caching schema (simulate ATLAS CORALCOOL-2930)
    def test161_atlas_coralcool2930(self):
        dbg = True
        if dbg: print("")
        self.internalTest16x_atlas_coralcool2930(dbg,1)

    # Test glitch after caching schema (simulate ATLAS CORALCOOL-2930)
    def test162_atlas_coralcool2930(self):
        dbg = True
        if dbg: print("")
        self.internalTest16x_atlas_coralcool2930(dbg,2)

    # Test glitch after caching transaction (simulate ATLAS CORALCOOL-2930)
    def test163_atlas_coralcool2930(self):
        dbg = True
        if dbg: print("")
        self.internalTest16x_atlas_coralcool2930(dbg,3)

    # Test glitch after starting transaction (simulate ATLAS CORALCOOL-2930)
    def test164_atlas_coralcool2930(self):
        dbg = True
        if dbg: print("")
        try:
            self.internalTest16x_atlas_coralcool2930(dbg,4)
            self.vFail( "Test #164 should throw" )
        except coral.Exception as error:
            print("Exception caught: '"+str(error)+"'")
            exp = "ORA-03113"
            if exp not in str(error):
                self.vFail( "Exception caught but does not contain '"+exp+"'" )
        print(msghdr(), "Exception caught as expected")

    # Test glitch after caching tableHandle (simulate ATLAS CORALCOOL-2930)
    def test165_atlas_coralcool2930(self):
        dbg = True
        if dbg: print("")
        self.internalTest16x_atlas_coralcool2930(dbg,5)

    # Catch exceptions in test 16x
    def internalTest16x_atlas_coralcool2930(self, dbg, i):
        try:
            self.internalTest16x_atlas_coralcool2930_wrapped( dbg, i )
        except Exception as e:
            print("ERROR!",e)
            raise e

    # Main driver for tests 16x
    def internalTest16x_atlas_coralcool2930_wrapped(self, dbg, i):
        if dbg: print(msghdr(), "Open a network tunnel")
        nt.open()
        if dbg: print(msghdr(), "Connect R/O to:", tunnelUrl)
        session = svc.connect(tunnelUrl, coral.access_Update )
        if i == 1:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before retrieving schema()")
        schema = session.nominalSchema()
        if i == 2:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch before retrieving transaction()")
        transaction = session.transaction()
        if i == 3:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch after retrieving a transaction")
        transaction.start(False)
        if i == 4:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch after starting a transaction")
        tableHandle = schema.tableHandle( tableName )
        if i == 5:
            if dbg: print(msghdr(), "GLITCH.....!")
            nt.glitch()
            if dbg: print(msghdr(), "GLITCH OVER!")
            if dbg: print(msghdr(), "Glitch after retrieving tableHandle()")
        dataEditor = tableHandle.dataEditor()
        print(msghdr(), "Successfully retrieved data editor for", tableName)

#============================================================================

if __name__ == '__main__':

    print("-"*70)

    # Build the unique table name and port number
    lport = 45000
    tableName = "NETWORKGLITCHTEST"
    tableNameUpdate = "NETWORKGLITCHTEST_UP"
    import PyCoralTest
    tableName = PyCoralTest.buildUniqueTableName( tableName )
    tableNameUpdate = PyCoralTest.buildUniqueTableName( tableNameUpdate )
    lport = PyCoralTest.buildUniquePortNumber()
    print(msghdr(), "Table name:", tableName)
    print(msghdr(), "Table name for update tests:", tableNameUpdate)
    print(msghdr(), "Local port number:", lport)

    # Do not let ORA-01466 data dictionary queries interfere (bug #92391)!
    # Always sleep 1s without such queries (that may fail with ORA-03113)
    if 'CORAL_TESTSUITE_SLEEPFOR01466' in os.environ:
        os.environ[ 'CORAL_TESTSUITE_SLEEPFOR01466' ] = "ALWAYS"

    # Create the connection string
    # ['lcg_coral_nightly_proxy' must be in $CORAL_AUTH_PATH/authentication.xml]
    if 'CORAL_AUTH_PATH' not in os.environ: # do not hardcode (CORALCOOL-2929)
        print("WARNING! CORAL_AUTH_PATH is not yet defined: set it")
        os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
    tnsalias = "lcg_coral_nightly_proxy"
    if 'CORAL_NETWORKGLITCHTEST_USER' not in os.environ: # CORALCOOL-2888
        tunnelUrl = "oracle://"+ tnsalias + "/lcg_coral_nightly"
    else:
        tunnelUrl = "oracle://"+ tnsalias + "/" + os.environ['CORAL_NETWORKGLITCHTEST_USER']

    # Create the appropriate tnsnames.ora
    tnsdir = "/tmp/" + os.environ['USER']
    if "BINARY_TAG" in os.environ:
        tnsdir += "/" + os.environ["BINARY_TAG"]
    if "SLOTNAME" in os.environ:
        tnsdir += "/" + os.environ["SLOTNAME"]
    os.system( "\\mkdir -p " + tnsdir )
    tnsfile = tnsdir + "/tnsnames.ora"
    print(msghdr(), "TNS_ADMIN:", tnsdir)
    os.environ['TNS_ADMIN'] = tnsdir
    # NB DB moved from test2 to intdb11 in Oct 2014 (test2 is still an alias)
    # NB The test2_nightly service uses only one instance (which may change
    # if DBAs forget to 'relocate' it, bug #103395) and the network glitch
    # test will fail if trying to connect to the wrong instance. To improve
    # reliability, use the intdb11_lb service (defined on both nodes) and
    # specify in tnsnames.ora one single INSTANCE_NAME and the corresponding
    # host name (itrac1302-v for intdb111, itrac1306-v for intdb112).
    tnsoutput = tnsalias+"=(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=localhost)(PORT="+str(lport)+")))(CONNECT_DATA=(SERVICE_NAME="+NetworkTunnel.svcnm()+")(INSTANCE_NAME="+NetworkTunnel.insnm()+")(SERVER=DEDICATED)))"
    fd = open(tnsfile, 'w')
    fd.write(tnsoutput)
    fd.close()

    # Extra verbosity controls for CORAL
    os.environ['CORAL_CONNECTIONPOOL_VERBOSE']='1'
    os.environ['CORAL_MSGFORMAT']='ATLAS'

    # Bootstrap CORAL
    ###os.environ['CORAL_MSGLEVEL']='Error'
    os.environ['CORAL_MSGLEVEL']='Warning'
    ###os.environ['CORAL_MSGLEVEL']='Info'
    ###os.environ['CORAL_MSGLEVEL']='Debug'
    ###os.environ['CORAL_MSGLEVEL']='Verbose'
    import coral
    msgLevel = coral.message_Level_Error
    if os.environ['CORAL_MSGLEVEL'] == 'Warning':
        msgLevel = coral.message_Level_Warning
    if os.environ['CORAL_MSGLEVEL'] == 'Info':
        msgLevel = coral.message_Level_Info
    if os.environ['CORAL_MSGLEVEL'] == 'Debug':
        msgLevel = coral.message_Level_Debug
    if os.environ['CORAL_MSGLEVEL'] == 'Verbose':
        msgLevel = coral.message_Level_Verbose
    rowBuffer = coral.AttributeList()
    rowBuffer.extend("ID","int")
    rowBuffer.extend("Data","float")
    print(msghdr(), "Instantiate the PyCoral connection service")
    svc = coral.ConnectionService()

    # Save CORAL default retrial parameters as global variables
    retrialPeriod=svc.configuration().connectionRetrialPeriod()
    retrialTimeOut=svc.configuration().connectionRetrialTimeOut()

    # Configure the connection service (fix bug #94103 in this test)
    # - disable the CORAL connection pool cleanup
    # - connection timeout=0: "idle" connections are immediately "expired"
    svc.configuration().disablePoolAutomaticCleanUp()
    svc.configuration().setConnectionTimeOut(0)

    # Open the network tunnel
    print(NetworkTunnel.tport())
    print(msghdr(), "Open a new network tunnel from localhost:" + str(lport) + " to " + NetworkTunnel.tport() + " through " + NetworkTunnel.ghost())
    nt = NetworkTunnel(lport)
    ###nt = NetworkTunnel(lport,debug=True)
    nt.open()

    # Recreate the RO table?
    doRecreateTable = False # default for interactive tests
    if "LCG_NGT_SLT_NUM" in os.environ:
        doRecreateTable = True # default for nightly tests
    if len(sys.argv) > 1 and sys.argv[1] == "-recreateTable":
        doRecreateTable = True # undocumented (argv[1] can also be a test)
        sys.argv.pop(1)
    print(msghdr(), "Create the table if needed")
    createTable( tableName, doRecreateTable )
    ###createTable( tableNameUpdate, doRecreateTable ) # NOT NEEDED!!

    # Start the unit test (can specify one specific test as cl argument)
    unittest.main( testRunner =
                   unittest.TextTestRunner(stream=sys.stdout,verbosity=2) )
