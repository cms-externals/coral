import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time
import pickle


try:

  ## Fill Data
  svc = coral.ConnectionService()
  session = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start()

  schema = session.nominalSchema()

  print("About to drop previous table")
  schema.dropIfExistsTable( "T" )

  print("Describing new table")
  description = coral.TableDescription()
  description.setName( "T" )


  description.insertColumn( "ID","int" )
  description.setPrimaryKey( "ID" )
  description.insertColumn( "TheDate","date" )
  description.insertColumn( "TheTime","time stamp", 6 )

  print("About to create the table")
  table = schema.createTable( description )

  rowBuffer = coral.AttributeList()
  table.dataEditor().rowBuffer( rowBuffer )

  for i in range(  0, 5 ):
    rowBuffer["ID"].setData(i)
    rowBuffer["TheDate"].setData( coral.Date( 2005, 11, i+1 ))
    rowBuffer["TheTime"].setData(coral.TimeStamp() )
    table.dataEditor().insertRow( rowBuffer )

  bulkInserter = table.dataEditor().bulkInsert( rowBuffer, 3 )
  fraction = 111111111

  for i in range(5,10): 
    fraction = fraction/10
    rowBuffer["ID"].setData(i)
    rowBuffer["TheDate"].setData(coral.Date( 2005, 11, i+1 ))
    thisMoment = coral.TimeStamp()
    rowBuffer["TheTime"].setData( coral.TimeStamp( thisMoment.year(), thisMoment.month(), thisMoment.day(), thisMoment.hour(), thisMoment.minute(), thisMoment.second(), (i-4) * fraction ))
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
  session = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start(True)

  schema = session.nominalSchema()

  query = schema.tableHandle("T").newQuery()
  query.setRowCacheSize( 20 )
  cursor = query.execute()
  i = 0
  while ( cursor.next() ):
    currentRow = cursor.currentRow()
    print(str(currentRow))
    i = i+1

  if ( i != 10 ):
    print("Unexpected number of rows")

  del query

  transaction.commit()
  del session

  print("SUCCESS IN READING DATA")

except Exception as e:
  print("FAILURE IN READING DATA")
  print(str(e))

