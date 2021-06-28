import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral


try:
  svc = coral.ConnectionService()
  session1 = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session1.transaction()
  transaction.start()

  ###### Tests for Schema related Methods
  schema = session1.nominalSchema()
  schema.dropIfExistsTable( 'MyTable' )
  listOfTableNames = schema.listTables()

  for tableName in listOfTableNames:
    print(tableName)
  for viewName in schema.listViews():
    print(viewName)
  ############################################

  ###### Tests for Table related Methods
  description = coral.TableDescription()
  description.setName("MyTable")
  description.insertColumn( 'ID', 'int' )
  description.insertColumn( 'Data', 'float' )
  description.setPrimaryKey( 'ID' )
  tableHandle = schema.createTable( description )

  print(tableHandle.description().name())
  ######################################################

  ############ Tests Related to ViewFactory & Views
  print('Check for Existence of View "MyView1" : ' , schema.existsView('MyView1'))
  if schema.existsView( 'MyView1' ):
    viewHandle1 = schema.viewHandle( 'MyView1' )
    print("View Definition = " , viewHandle1.definition())
    schema.dropView( 'MyView1' )
    
  viewFactory = schema.viewFactory()
  viewFactory.addToTableList( 'MyTable' )
  viewFactory.addToOutputList( 'ID' )
  print('Creating view')
  view1 = viewFactory.create('MyView1')
  print('View created')
  
  print("Check for Existence of View Name = " , schema.existsView('MyView1'))
  print("View Name = " , view1.name())
  print("View Definition = " , view1.definition())
  print("View NumberOfColumns = " , view1.numberOfColumns())


  transaction.commit()
  del session1

except Exception as e:
  print(str(e))
