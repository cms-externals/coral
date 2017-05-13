import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import SchemaTest
import MySqlSchema


try: 
 #Initialize Connection Service
 svc = coral.ConnectionService() 
 config = svc.configuration()
 config.setConnectionRetrialPeriod(2)
 config.setConnectionRetrialTimeOut(20)    

 #open session proxy using MySQL technology providing logical service name & access mode

 proxy1 = svc.connect("my_test",coral.access_ReadOnly);

 print "Test with mysql."
 mysqlTest = MySqlSchema.MySqlSchema(proxy1)
 mysqlTest.listTables("test")

 #connection exists and will be recycled */
 proxy_update = svc.connect("my_test",coral.access_Update)
 mysqlTest_update = MySqlSchema.MySqlSchema(proxy_update)
 mysqlTest_update.listTables("RADO_POOL")
 #print " Is connection shared: " , proxy_update.isConnectionShared()

 del proxy1
 del proxy_update

 #open session proxy using SQLite technology

 proxy1 = svc.connect("sql_test",coral.access_Update)
 print "Supported C++ types: "
 cppTypes = proxy1.typeConverter().supportedCppTypes()
 for iType in cppTypes:
  print iType
  print "                      -> "
  print proxy1.typeConverter().sqlTypeForCppType( iType )

 del proxy1

    
 print "Schema Test - writer."
 #get proxy to session providing logical service name and access mode
 proxy = svc.connect("oracle://lcg_coral_nightly/lcg_coral_nightly",coral.access_Update)
 #create and fill schema using same authentication parameters
 test1 = SchemaTest.SchemaTest(proxy)
 test1.createSchema()
 del test1
 del proxy
       
 print "Schema Test - reader."
 #read created schema using new authentication on previously opened
 #   and released session - authentication parameters will be switched 
 #proxy2 = svc.connect("reader_test",coral.access_ReadOnly)
 proxy2 = svc.connect("oracle://lcg_coral_nightly/lcg_coral_nightly",coral.access_ReadOnly)
 schema = proxy2.nominalSchema()
 test2 = SchemaTest.SchemaTest(proxy2)
 test2.readSchema("lcg_coral_nightly")
 del test2
 del proxy2
 print "Test completed."



except Exception, e:
 print "Test FAILED"
 print str(e)

