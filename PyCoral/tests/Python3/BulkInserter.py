import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time
import pickle


try:

  ## Fill Data
  svc = coral.ConnectionService()
  session = svc.connect( 'write_test', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start()

  schema = session.nominalSchema()

  print("About to drop previous table")
  schema.dropIfExistsTable( "T" )

  print("Describing new table")
  description = coral.TableDescription()
  description.setName( "T" )
  description.insertColumn( "ID","int")
  description.setPrimaryKey( "ID" );
  description.insertColumn( "x","float")
  description.setNotNullConstraint( "x" )
  description.insertColumn( "Y","double")
  description.insertColumn( "Z","double")
  description.insertColumn( "data1","string", 100, False )
  description.insertColumn( "data2", "blob")

  print("About to create the table")
  table = schema.createTable( description )

  rowBuffer = coral.AttributeList()
  table.dataEditor().rowBuffer( rowBuffer )
  bulkInserter = table.dataEditor().bulkInsert( rowBuffer, 3 )

  for i in range (0,5):
    rowBuffer["ID"].setData(i)
    t1 = i + 0.1*i
    rowBuffer["x"].setData(t1)
    t2 = i + 0.01*i
    rowBuffer["Y"].setData(t2)
    if ( i % 2 == 1 ):
      rowBuffer["Y"].setData(None)
    t3 = i + 0.001*i
    rowBuffer["Z"].setData(t3)
    rowBuffer["data1"].setData("Data")
    rowBuffer["data2"].setData("Data2")

    bulkInserter.processNextIteration() 

  bulkInserter.flush()
  del bulkInserter

  transaction.commit()
  del session

  print("SUCCESS IN FILLING DATA")

except Exception as e:
  print("FAILURE IN FILLING DATA")
  print(str(e))

print("Sleeping for 1 second")
time.sleep(1)

try:

  ## Read Data
  svc = coral.ConnectionService()
  session = svc.connect( 'my_test', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start(True)

  schema = session.nominalSchema()
  query = schema.tableHandle("T").newQuery()

  cursor = query.execute()
  i = 0
  while ( cursor.next() ):
    currentRow = cursor.currentRow()
    print(str(currentRow)) 
  del query;

  transaction.commit()
  del session

  print("SUCCESS IN READING DATA")

except Exception as e:
  print("FAILURE IN READING DATA")
  print(str(e))

