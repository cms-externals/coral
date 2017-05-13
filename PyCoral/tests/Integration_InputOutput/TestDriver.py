import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import pickle
import math

tableName = "CORAL_TESTING_T_1"
rowsWithSimpleInsert = 10
rowsWithBulkInsert = 55
m_rowBuffer = coral.AttributeList()

def prepareSchema( serviceName ):
 try:
  svc = coral.ConnectionService()
  connectionName = '/dbdev/service_'+serviceName
  print connectionName
  session = svc.connect( connectionName = connectionName, accessMode = coral.access_Update )

  transaction = session.transaction()
  transaction.start()
  schema = session.nominalSchema()

  # Clear the old table
  schema.dropIfExistsTable( tableName )

  global m_rowBuffer

  print type(m_rowBuffer)
  if ( m_rowBuffer ):
   del m_rowBuffer

  m_rowBuffer = coral.AttributeList()
  rowBuffer = m_rowBuffer

  # Create the new table
  description = coral.TableDescription()
  description.setName( tableName )

  description.insertColumn( "ID","int" )
  description.setNotNullConstraint( "ID" )
  description.setPrimaryKey( "ID" )
  rowBuffer.extend( "ID","int" )

  description.insertColumn( "V_B","bool" )
  rowBuffer.extend( "V_B","bool" )

  description.insertColumn( "V_UC","unsigned char" )
  rowBuffer.extend( "V_UC","unsigned char" )

  description.insertColumn( "V_SC","char" )
  rowBuffer.extend( "V_SC","char" )

  description.insertColumn( "V_US","unsigned short" )
  rowBuffer.extend( "V_US","unsigned short" )

  description.insertColumn( "V_SS","short" )
  rowBuffer.extend( "V_SS", "short" )

  description.insertColumn( "V_UI","unsigned int" )
  rowBuffer.extend( "V_UI","unsigned int" )

  description.insertColumn( "V_SI","int" )
  rowBuffer.extend( "V_SI","int" )

  description.insertColumn( "V_UL","unsigned long" )
  rowBuffer.extend( "V_UL","unsigned long" )

  description.insertColumn( "V_SL","long" )
  rowBuffer.extend( "V_SL","long" )

  description.insertColumn( "V_ULL","unsigned long long" )
  rowBuffer.extend( "V_ULL","unsigned long long" )

  description.insertColumn( "V_SLL","long long" )
  rowBuffer.extend( "V_SLL","long long" )

  description.insertColumn( "V_F","float" )
  rowBuffer.extend( "V_F","float" )

  description.insertColumn( "V_D","double" )
  rowBuffer.extend( "V_D","double" )

  description.insertColumn( "V_LD","double" )
  rowBuffer.extend( "V_LD","double" )

  description.insertColumn( "V_BLOB","blob" )
  rowBuffer.extend( "V_BLOB","blob" )

  description.insertColumn( "V_DATE","date" )
  rowBuffer.extend( "V_DATE","date" )

  description.insertColumn( "V_TIMESTAMP","time stamp" )
  rowBuffer.extend( "V_TIMESTAMP","time stamp" )

  description.insertColumn( "V_SG","string" )
  rowBuffer.extend( "V_SG","string" )

  description.insertColumn( "V_SF","string", 5 )
  rowBuffer.extend( "V_SF","string" )

  description.insertColumn( "V_SV","string", 50, False )
  rowBuffer.extend( "V_SV","string" )

  schema.createTable( description )

  transaction.commit()
  del session
  print "PrepareSchema SUCCESS for /dbdev/service_" , serviceName 

 except Exception, e:
  raise Exception( "PrepareSchema FAILURE for /dbdev/service_" + " " + serviceName + " " + str(e) )


def fillData( i ):
 try:
  global m_rowBuffer

  row = m_rowBuffer
  row[0].setData(i)
  if ( i%3 == 0 ):
   row[1].setData( None )
  else:
   if ( i%2 == 0 ):
    temp = True
   else: 
    temp = False
   row[1].setData(temp)

  row[2].setData(( i*2 ) % 256)

  row[3].setData(( i*2 ) % 128)

  row[4].setData(( i*3 ) % 2048)

  row[5].setData(( i*3 ) % 2048 - 1024)

  row[6].setData(( i*4 ) % 1000)

  row[7].setData(( i*4 ) % 1000 - 500)

  row[8].setData(( i*4 ) % 1001)

  row[9].setData(( i*4 ) % 1001 - 500)

  row[10].setData(i % 123456789)

  row[11].setData(i % 123456789 - 500)

  if ( i%4 == 0 ):
   row[12].setData( None )
  else:
   row[12].setData(i + 0.123) 

  row[13].setData( 0.123456789 - 2.3*i) 

  row[14].setData(0.123456789 + 2.3*i) 

  blob = coral.Blob()
  row[15].setData(blob)
  blobSize = 1000 * ( i%100 + 1 )
  #blob.resize( blobSize );
  p = []
  for j in range( 0, blobSize ):
   p.append(( i + j )%256)

  pickle.dump(p,blob,True)
  row[15].setData(blob)

  row[16].setData(coral.Date( 2006, 1, i%31 + 1 ))

  row[17].setData(coral.TimeStamp( 2006, 1, 12, 15, 47, i%60, 0 ))

  os1 = "A general String : " + str( i % 5)
  s1 = str(os1)
  row[18].setData(s1)

  os2 = "...." + str(i % 10)
  s2 = str(os2)
  row[19].setData(s2)

  os3 = "Value : " + str( i)
  s3 = str(os3)
  row[20].setData(s3)

  print "FillData SUCCESS " 

 except Exception, e:
  raise Exception( "FillData FAILURE " + str(e) )


def checkData( i ):
 try:
  global m_rowBuffer

  row = m_rowBuffer

  if ( row[0].data() != i ):
   raise Exception( "Unexpected value for variable " , row[0].specification().name())

  if ( i%3 == 0 ):
    if ( not row[1].isNull() ):
     raise Exception( "Unexpected NOTNULL data for variable ",row[1].specification().name())
  else:
    if ( row[1].isNull() ):
     raise Exception( "Unexpected NULL data for variable ",row[1].specification().name())
    if ( i%2 == 0 ):
     temp = True
    else:
     temp = False
    if ( row[1].data() != temp):
     raise Exception( "Unexpected value for variable " ,row[1].specification().name())

  if ( row[2].data() != ( i*2 ) % 256 ):
   raise Exception( "Unexpected value for variable " , row[2].specification().name())

  if ( ord(row[3].data()) != ( i*2 ) % 128 ):
   raise Exception( "Unexpected value for variable " , row[3].specification().name())

  if ( row[4].data() != ( i*3 ) % 2048 ):
   raise Exception( "Unexpected value for variable " , row[4].specification().name())

  if ( row[5].data() != ( i*3 ) % 2048 - 1024 ):
   raise Exception( "Unexpected value for variable " , row[5].specification().name())

  if ( row[6].data() != ( i*4 ) % 1000 ):
   raise Exception( "Unexpected value for variable " , row[6].specification().name())

  if ( row[7].data() != ( i*4 ) % 1000 - 500 ):
   raise Exception( "Unexpected value for variable " , row[7].specification().name())

  if ( row[8].data() != ( i*4 ) % 1001 ):
   raise Exception( "Unexpected value for variable " , row[8].specification().name())

  if ( row[9].data() != ( i*4 ) % 1001 - 500 ):
   raise Exception( "Unexpected value for variable " , row[9].specification().name())

  if ( row[10].data() != i % 123456789 ):
   raise Exception( "Unexpected value for variable " , row[10].specification().name())

  if ( row[11].data() != i % 123456789 - 500 ):
   raise Exception( "Unexpected value for variable " , row[11].specification().name())

  if ( i%4 == 0 ):
   if ( not row[12].isNull() ):
    raise Exception( "Unexpected NOTNULL data for variable ",row[12].specification().name())
  else:
   if ( row[12].isNull() ):
    raise Exception( "Unexpected NULL data for variable ",row[12].specification().name())
    if ( abs( row[12].data() / ( i + 0.123 ) - 1 ) > 1e-324 ):
     raise Exception("Unexpected value for variable " , row[12].specification().name())

  if ( abs( row[13].data() / ( 0.123456789 - 2.3*i ) - 1 ) > 1e-324 ):
   raise Exception( "Unexpected value for variable " , row[13].specification().name())

  if ( abs( row[14].data() / ( 0.123456789 + 2.3*i ) - 1 ) > 1e-324 ):
   raise Exception( "Unexpected value for variable " , row[14].specification().name())

  blob = row[15].data()
  p1 = pickle.load(blob)
  blobSize = 1000 * ( i%100 + 1 );
  if ( len(p1) != blobSize ):
   raise Exception( "Unexpected blob size for variable " , row[15].specification().name())
  for j in range( 0, blobSize ):
   if ( p1[j] != ( i + j )%256 ):
    raise Exception( "Unexpected value for variable " , row[15].specification().name())

  if ( row[16].data() != coral.Date( 2006, 1, i%31 + 1 ) ):
   raise Exception( "Unexpected value for variable " , row[16].specification().name())
  
  if ( row[17].data() != coral.TimeStamp( 2006, 1, 12, 15, 47, i%60, 0 ) ):
   raise Exception( "Unexpected value for variable " , row[17].specification().name())

  os1 = "A general String : " + str(i % 5)
  s1 = str(os1)
  if ( row[18].data() != s1 ):
   raise Exception ( "Unexpected value for variable " + row[18].specification().name())

  os2 = "...." + str(i % 10)
  s2 = str(os2)
  if ( row[19].data() != s2 ):
   raise Exception( "Unexpected value for variable ", row[19].specification().name())

  os3 = "Value : " + str(i)
  s3 = str(os3)
  if ( row[20].data() != s3 ):
   raise Exception( "Unexpected value for variable " , row[20].specification().name())

  print "CheckData SUCCESS "

 except Exception, e:
  raise Exception( "CheckData FAILURE " + str(e) )



def writeSimple( serviceName ):
 try:
  svc = coral.ConnectionService()
  connectionName = '/dbdev/service_'+serviceName
  print connectionName
  session = svc.connect( connectionName = connectionName, accessMode = coral.access_Update )

  transaction = session.transaction()
  transaction.start()
  schema = session.nominalSchema()

  table = schema.tableHandle( tableName )
  dataEditor = table.dataEditor()
  global m_rowBuffer

  rowBuffer = m_rowBuffer

  for i in range( 0, rowsWithSimpleInsert):
   fillData( i )
   dataEditor.insertRow( rowBuffer )

  transaction.commit()
  del session
  print "WriteSimple SUCCESS for /dbdev/service_" , serviceName 

 except Exception, e:
  raise Exception( "WriteSimple FAILURE for /dbdev/service_" + " " + serviceName + " " + str(e) )


def writeBulk( serviceName ):
 try:
  svc = coral.ConnectionService()
  connectionName = '/dbdev/service_'+serviceName
  print connectionName
  session = svc.connect( connectionName = connectionName, accessMode = coral.access_Update )

  transaction = session.transaction()
  transaction.start()
  schema = session.nominalSchema()

  table = schema.tableHandle( tableName )
  dataEditor = table.dataEditor()
  global m_rowBuffer
  rowBuffer = m_rowBuffer

  bulkOperation = dataEditor.bulkInsert( rowBuffer, rowsWithSimpleInsert )

  for i in range( rowsWithSimpleInsert,rowsWithSimpleInsert + rowsWithBulkInsert):
   fillData( i )
   bulkOperation.processNextIteration()

  bulkOperation.flush()

  del bulkOperation;

  transaction.commit()
  del session
  print "WriteBulk SUCCESS for /dbdev/service_" , serviceName 
 except Exception, e:
  raise Exception( "WriteBulk FAILURE for /dbdev/service_" + " " + serviceName + " " + str(e) )


def read( serviceName ):
 try:
  svc = coral.ConnectionService()
  connectionName = '/dbdev/service_'+serviceName
  print connectionName
  session = svc.connect( connectionName = connectionName, accessMode = coral.access_ReadOnly )
  transaction = session.transaction()
  transaction.start(True)

  schema = session.nominalSchema()

  table = schema.tableHandle( tableName )
  query = table.newQuery()
  query.addToOrderList( "ID" )
  global m_rowBuffer
  query.defineOutput( m_rowBuffer )
  query.setRowCacheSize( rowsWithSimpleInsert )

  cursor = query.execute()
  for i in range( 0,rowsWithSimpleInsert + rowsWithBulkInsert ):
    if ( not cursor.next() ):
      raise Exception( "Unexpected number of returned rows")
    checkData( i )

  if ( cursor.next() ):
    raise Exception( "Unexpected number of returned rows" )

  del query

  del session
  print "Read SUCCESS for /dbdev/service_" , serviceName 

 except Exception, e:
  raise Exception( "Read FAILURE for /dbdev/service_" + " " + serviceName + " " + str(e) )
