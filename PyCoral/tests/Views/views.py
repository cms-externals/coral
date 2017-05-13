import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time


try:
  svc = coral.ConnectionService()
  #session1 = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  session1 = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session1.transaction()
  transaction.start()
 
  schema = session1.nominalSchema();

  print "About to drop previous tables and views" 
  schema.dropIfExistsView( "VT" )
  schema.dropIfExistsTable( "T" )

  print "Creating a new table"
  description = coral.TableDescription()
  description.setName( "T" );
  description.insertColumn( "ID","int")
  description.setPrimaryKey( "ID" );
  description.insertColumn( "x","float")
  description.setNotNullConstraint( "x" )
  description.insertColumn( "Y","double")
  schema.createTable( description )

  print "About to create a view"
  factory = schema.viewFactory()
  factory.addToOutputList( "tt.x", "x" )
  factory.addToOutputList( "tt.Y", "y" )
  factory.addToTableList( "T", "tt" )
  factory.setCondition( "ID > 2", coral.AttributeList() )
  factory.create( "VT" )

  del factory

  session1.transaction().commit()

  time.sleep(1)

  session1.transaction().start( 1 )

  view = session1.nominalSchema().viewHandle( "VT" )
  print "View definition : " , view.definition()

  numberOfColumns = view.numberOfColumns()
  for i in range (0, numberOfColumns ):
    column = view.column( i )
    print "  " , column.name(), " (" , column.type() , ")"
    if ( column.isNotNull() ): 
     print " NOT NULL"
    print "\n"

  transaction.commit()
  del session1

  print "SUCCESS"
except Exception, e:
  print "FAILURE"
  print str(e)

