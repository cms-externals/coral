#!/usr/bin/env python
import os, sys, unittest
from time import sleep
import PyCoralTest

#============================================================================

def createTable():
    ###print MSGHDR, "Recreate the table"
    session = svc.connect( urlRW, coral.access_Update )
    session.transaction().start(False)
    session.nominalSchema().dropIfExistsTable( tableName )
    description = coral.TableDescription()
    description.setName( tableName)
    description.insertColumn( 'ID', 'int' )
    description.insertColumn( 'Data', 'float' ) # MySQL test fails with double?
    description.setPrimaryKey( 'ID' )
    tableHandle = session.nominalSchema().createTable( description )
    ###print MSGHDR, "Fill the table"
    bulkInserter = tableHandle.dataEditor().bulkInsert( rowBuffer, 100 )
    for i in range(1000):
        rowBuffer["ID"].setData(i)
        rowBuffer["Data"].setData(i)
        bulkInserter.processNextIteration()
    bulkInserter.flush()
    if urlRW == PyCoralTest.buildUrl( "Oracle", False ):
        ###print MSGHDR, "Grant SELECT on table to public"
        tableHandle.privilegeManager().grantToUser( "PUBLIC",
                                                    coral.privilege_Select );
    session.transaction().commit()
    sleep(1)

#============================================================================

class PyCoralBasicTest( PyCoralTest.TestCase ):

    #------------------------------------------------------------------------

    def setUp(self):
        # Call the base class method
        PyCoralTest.TestCase.setUp(self)
        print("")

    #------------------------------------------------------------------------

    def tearDown(self):
        # Call the base class method
        PyCoralTest.TestCase.tearDown(self)
            
    #------------------------------------------------------------------------

    def test010_simpleQuery(self):
        session = svc.connect( urlRO, coral.access_ReadOnly )
        session.transaction().start(True)
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        counter = 0
        while ( counter < 5 and cursor.next() ):
            self.assertEqual( counter, rowBuffer[0].data() )
            self.assertEqual( counter, rowBuffer[1].data() )
            counter += 1
        session.transaction().commit()

    # WHAT IS THIS? TO BE COMPLETED? (enable it anyway - it cannot harm) 
    def test010a_simpleQuery_readAllRows(self):
        session = svc.connect( urlRO, coral.access_ReadOnly )
        session.transaction().start(True)
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        counter = 0
        print(MSGHDR, "Connected, get all rows")
        while ( cursor.next() ): # Read ALL rows... this should succeed
            self.assertEqual( counter, rowBuffer[0].data() )
            self.assertEqual( counter, rowBuffer[1].data() )
            counter += 1
        session.transaction().commit()
        print(MSGHDR, "Got all rows and committed, now disconnect")
        session = 0
        svc.purgeConnectionPool()
        sleep(2)
        print(MSGHDR, "Disconnected(?)")        

    # WHAT IS THIS? TO BE COMPLETED? (enable it anyway - it cannot harm) 
    def test010b_simpleQuery_CoralServer_bug73834(self):
        session = svc.connect( urlRO, coral.access_ReadOnly )
        session.transaction().start(True)
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        counter = 0
        print(MSGHDR, "Connected, get first 5 rows")
        while ( counter < 5 and cursor.next() ):
            self.assertEqual( counter, rowBuffer[0].data() )
            self.assertEqual( counter, rowBuffer[1].data() )
            counter += 1
        session.transaction().commit()
        # This crashes on the CoralServer (bug #73834) because the session is
        # closed after reading the first 5 rows but the CoralServer will
        # continue to send the remaining 5 rows (10 in total) in FetchAllRows.
        # [The '10 in total' was true on version 1.5, now there are 1000 rows.]
        print(MSGHDR, "Got only 5 rows and committed, now disconnect")
        session = 0
        svc.purgeConnectionPool()
        sleep(2)
        print(MSGHDR, "Disconnected(?)")        

    #------------------------------------------------------------------------

    # Test for bug #75094 aka CORALCOOL-952
    def test011_bug75094_nextAfterCommittingTransaction(self):
        #if urlRO != PyCoralTest.buildUrl( "Oracle", True ) \
        #       and urlRO != PyCoralTest.buildUrl( "MySQL", True ) \
        #       and "ENABLE_TEST_BUG75094" not in os.environ:
        #    print "Skip test (ENABLE_TEST_BUG75094 not set)"
        #    return # disabled (it fails)
        session = svc.connect( urlRO, coral.access_ReadOnly )
        ###print 'Start the transaction'
        session.transaction().start(True)
        ###print 'Is the transaction active?', session.transaction().isActive()
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        counter = 0
        print(MSGHDR, "Connected, get first 5 rows")
        while ( counter < 5 and cursor.next() ):
            ###print 'Current row', rowBuffer
            self.assertEqual( counter, rowBuffer[0].data() )
            self.assertEqual( counter, rowBuffer[1].data() )
            counter += 1
        ###print 'Commit the transaction'
        session.transaction().commit()
        # Fixed bug #75094: cursor.next should throw (transaction not active)
        ###print 'Is the transaction active?', session.transaction().isActive()
        self.assertRaises( Exception, lambda: cursor.next() ) # see bug #75096
        print(MSGHDR, "Got exception as expected, now disconnect")
        session = 0
        cursor = 0 # needed to disconnect (?)
        query = 0 # needed to disconnect (?)
        svc.purgeConnectionPool() # is this needed?
        print(MSGHDR, "Disconnected(?)")        

    #------------------------------------------------------------------------

    # WHAT IS THIS? TO BE COMPLETED? (enable it anyway - it cannot harm) 
    def test012_bug73834_nextAfterClosingSession(self):
        coral.MessageStream("").setMsgVerbosity(coral.message_Level_Info)
        session = svc.connect( urlRO, coral.access_ReadOnly )
        ###print MSGHDR, 'Start the transaction'
        session.transaction().start(True)
        ###print MSGHDR, 'Is tx active?', session.transaction().isActive()
        query = session.nominalSchema().tableHandle(tableName).newQuery()
        query.addToOrderList( "ID ASC" )
        query.defineOutput( rowBuffer )
        query.setMemoryCacheSize(0)
        query.setRowCacheSize(1)
        cursor = query.execute()
        counter = 0
        print(MSGHDR, 'Get first 5 rows')
        while ( counter < 5 and cursor.next() ):
            ###print MSGHDR, 'Current row', rowBuffer
            self.assertEqual( counter, rowBuffer[0].data() )
            self.assertEqual( counter, rowBuffer[1].data() )
            counter += 1
        session.transaction().commit()
        print(MSGHDR, "Got 5 rows, committed, now disconnect")
        session = 0
        ###query = 0
        ###print MSGHDR, 'Sleep 3s'
        sleep(3)
        ###print MSGHDR, 'Purge the connection pool'
        svc.purgeConnectionPool()
        print(MSGHDR, 'Purged the connection pool')
        ###print 'Is the transaction active?', session.transaction().isActive()
        # This may crash if the cursor uses a deleted session (bug #73834)?
        # Here cursor.next should throw (session is not active)
        if urlRO != PyCoralTest.buildUrl( "Oracle", True ) \
               and urlRO != PyCoralTest.buildUrl( "MySQL", True ) \
               and "ENABLE_TEST_BUG75094" not in os.environ:
            print("Skip test (ENABLE_TEST_BUG75094 not set)")
            return # disabled (it fails)
        self.assertRaises( Exception, lambda: cursor.next() ) # see bug #75096
    
#============================================================================

if __name__ == '__main__':

    print("")
    MSGHDR = "+++ PYCORAL TEST +++"

    # Build the unique table name and the URLs
    tableName = "PYCORALBASICTEST"
    import PyCoralTest
    tableName = PyCoralTest.buildUniqueTableName( tableName )
    [urlRW,urlRO] = PyCoralTest.parseArguments()
    print(MSGHDR, "Table name:", tableName)
    print(MSGHDR, "URL [RW,RO]:", [urlRW,urlRO])

    # Bootstrap CORAL
    #os.environ['CORAL_MSGLEVEL']='Verbose'
    #os.environ['CORAL_MSGLEVEL']='Info'
    import coral
    print(MSGHDR, "Instantiate the PyCoral connection service")
    svc = coral.ConnectionService()

    # Configure the connection service (see bug #71449)
    # - disable the CORAL connection pool cleanup
    # - connection timeout=0: "idle" connections become immediately "expired"
    svc.configuration().disablePoolAutomaticCleanUp()
    svc.configuration().setConnectionTimeOut(0)
    
    # Prepare the row buffer for this test
    rowBuffer = coral.AttributeList()
    rowBuffer.extend("ID","int")
    rowBuffer.extend("Data","float")

    # Recreate the table
    print(MSGHDR, "Create the test table")
    createTable()

    # Start the unit test (can specify one specific test as cl argument)
    print(MSGHDR, "Start the test suite")
    unittest.main( testRunner =
                   unittest.TextTestRunner(stream=sys.stdout,verbosity=2) )
