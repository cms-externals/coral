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
    session.transaction().commit()
    sleep(1)

#============================================================================

class PyCoralMiscellaneousBugsTest( PyCoralTest.TestCase ):

    #------------------------------------------------------------------------

    def setUp(self):
        # Call the base class method
        PyCoralTest.TestCase.setUp(self)
        print("")
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

    #------------------------------------------------------------------------

    def tearDown(self):
        # Purge the connection pool after each test
        svc.purgeConnectionPool()
        # Call the base class method
        PyCoralTest.TestCase.tearDown(self)
            
    #------------------------------------------------------------------------

    # Test bug #61090 aka bug #76501
    def test010_bug61090(self):
        session = svc.connect( urlRW, coral.access_Update )
        # Fill table in R/W tx (do not delete bulkInserter yet)
        print(MSGHDR, "Fill the table in R/W tx - do not delete bulkInserter")
        session.transaction().start(False)
        editor=session.nominalSchema().tableHandle(tableName).dataEditor()
        editor.deleteRows("",coral.AttributeList())
        bulkInserter = editor.bulkInsert( rowBuffer, 100 )
        for i in range(5):
            rowBuffer["ID"].setData(i)
            rowBuffer["Data"].setData(i+0.1*i)
            bulkInserter.processNextIteration()
        bulkInserter.flush()
        session.transaction().commit()
        # Read data in R/O tx (no need to reconnect R/O session to show bug)
        print(MSGHDR, "Query the table in R/O tx")
        session.transaction().start(True)
        query = session.nominalSchema().newQuery()
        query.addToTableList(tableName)
        query.setRowCacheSize(3)
        query.defineOutput(rowBuffer)
        cursor=query.execute()
        nrows = 0
        while cursor.next() :
            nrows = nrows + 1
            row = cursor.currentRow()
            print(MSGHDR, "Current row:", row)
        self.assertEqual( 5, nrows )
        cursor=None
        query=None
        session.transaction().commit()
        # Release the bulk operation outside a tx
        print(MSGHDR, "Release bulkInserter outside tx")
        crash=True
        ###crash=False # Flag to cause the crash or bypass it
        if not crash: session.transaction().start(True)
        bulkInserter = None # CRASH!
        if not crash: session.transaction().commit()

#============================================================================

if __name__ == '__main__':

    print("")
    MSGHDR = "+++ PYCORAL MIXBUGS TEST +++"

    # Build the unique table name and the URLs
    tableName = "PYCORALMIXBUGSTEST"
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

    # Save CORAL default retrial parameters as global variables
    retrialPeriod=svc.configuration().connectionRetrialPeriod()
    retrialTimeOut=svc.configuration().connectionRetrialTimeOut()

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
