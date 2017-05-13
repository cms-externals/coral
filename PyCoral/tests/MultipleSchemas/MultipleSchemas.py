import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import inspect


try:
  svc = coral.ConnectionService()
  session = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start(True)

  print "Tables in the nominal schema:" 
  listOfTables = session.nominalSchema().listTables()
  for iTable in listOfTables:
    print iTable

  print "Tables in the other schema:"
  try:
   listOfTables = session.schema( m_otherSchema ).listTables()
   for iTable in listOfTables:
    print iTable 
   print "No Schema Exception not caught"
  except Exception, e:
   print "No Schema Exception caught"
   print str(e)

  print "Attempting to catch a specific exception for accessing an unknown schema"

  transaction.commit()
  del session

  print "SUCCESS"

except Exception, e:
  print "FAILURE"
  print str(e)

