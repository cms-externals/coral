import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral


try:
  svc = coral.ConnectionService()
  session1 = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session1.transaction()
  transaction.start(True)

  schema = session1.nominalSchema()


  data = coral.AttributeList()
  data.extend( "X", "float" )

  query = schema.newQuery()
  query.addToOutputList( "3.21", "X" )
  query.defineOutput( data )
  query.addToTableList( "DUAL" )
  cursor = query.execute()
  cursor.next()
  print "X : " , data["X"].data()

  del query;



  transaction.commit()
  del session1

  print "SUCCESS"
except Exception, e:
  print str(e)
  print "FAILURE"

