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

  print "About to drop previous table"
  #schema.dropIfExistsTable( "T" )

  #print "write_test", accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start(True)

  schema = session.nominalSchema()
  query = schema.tableHandle("T").newQuery()

  #cursor = query.execute()
  i = 0

  #while ( cursor.next() ):
  #for currentRow in cursor:
  #for currentRow in query.execute():
  for currentRow in iter(query.execute()):
    #currentRow = cursor.currentRow()
    print str(currentRow) 
    blobAttribute = currentRow["data2"]
    if ( not blobAttribute.isNull() ) :
      blob = blobAttribute.data() 
      print "BLOB"
      print blob
      #num1 = pickle.loads(blob.readline())
      num1 = pickle.load(blob)
      #num1 = blob.readline()
      print type(num1)
      print "1........"
      blobSize = ( i + 1 ) * 1000
      if ( len(num1) != blobSize ):
        print "Unexpected blob size"
      for j in range (0, blobSize ):
        if ( num1[j] != j % 256 ):
          print "Unexpected blob data"
    i = i + 1

  if ( i != 5 ):
    print "Unexpected number of rows"

  del query;

  transaction.commit()
  del session

  print "SUCCESS IN READING DATA"

except Exception, e:
  print "FAILURE IN READING DATA"
  print str(e)

