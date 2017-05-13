import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral


try:
  svc = coral.ConnectionService()
  session = svc.connect( '/dbdev/service_1', accessMode = coral.access_Update )
  transaction = session.transaction()
  transaction.start()
  
  schema = session.nominalSchema()

  print "About to drop previous tables"
  schema.dropIfExistsTable( "T_1" )
  schema.dropIfExistsTable( "T_0" )

  print "Describing new tables"
  description0 = coral.TableDescription();
  description0.setName( "T_0" )
  description0.insertColumn( "ID","long" )
  description0.setUniqueConstraint( "ID" )
  description0.insertColumn( "X","float" )
  description0.insertColumn( "Y","float" )
  cols = ("X","Y")
  description0.createIndex( "T_0_IDX",cols )

  description = coral.TableDescription()
  description.setName( "T_1" )
  description.insertColumn( "ID","long" )
  description.setPrimaryKey( "ID" )
  description.insertColumn( "x","float")
  description.setNotNullConstraint( "x" )
  description.insertColumn( "Y","double" )
  description.setUniqueConstraint( "Y" )
  description.insertColumn( "Z","double" )
  description.insertColumn( "TheDate","date" )
  description.insertColumn( "TheTime","time stamp", 3 )

  constraintColumns = ("x","Z")
  description.setUniqueConstraint( constraintColumns, "C_T1" )
  description.insertColumn( "data1","string" , 123, True )
  description.insertColumn( "data2","string" , 321, False )
  description.insertColumn( "ID_FK","long" )
  description.createForeignKey( "T_1_FK", "ID_FK", "T_0", "ID" )

  print "About to create the tables" 
  table0 = schema.createTable( description0 )
  table0.privilegeManager().grantToPublic( coral.privilege_Select )
  table0.schemaEditor().renameColumn( "Y", "Z" )

  table = schema.createTable( description )

  tableSpaceName = table.description().tableSpaceName()
  print "Table T_1 created under the table space " , tableSpaceName 

  print "Retrieving the table description."

  print "Description : "
  numberOfColumns = table.description().numberOfColumns()
  for i in range(0,numberOfColumns,1):
    column = table.description().columnDescription( i )
    print column.name() , " : " , column.type()
    if ( column.isNotNull() ):
     print " NOT NULL"
    if ( column.isUnique() ):
     print  " UNIQUE"
    print

  if ( table.description().hasPrimaryKey() ):
    pk = table.description().primaryKey()
    print "Table has primary key defined in tablespace " , pk.tableSpaceName(), " for column(s) "
    for iColumn in pk.columnNames():
     print iColumn

  print "Foreign keys defined:"
  for i in range(0,table.description().numberOfForeignKeys() ):
    fkey = table.description().foreignKey( i )
    print "   " , fkey.name() , " ( "
    for iColumn in fkey.columnNames():
     print iColumn
    print " ) -> " , fkey.referencedTableName() , " ( "
    for iColumn in fkey.referencedColumnNames():
     print iColumn
    print " )"

  session.transaction().commit();
  session.transaction().start( True );

  print "Indices for Table T_0 :" 
  table00 = schema.tableHandle( "T_0" )
  for i in range(0,table00.description().numberOfIndices() ): 
    index = table00.description().index( i )
    print " " , index.name() , " -> "
    for iColumn in index.columnNames():
     print iColumn
    print " (tablespace : " , index.tableSpaceName() , ")"

  transaction.commit()
  del session

  print "SUCCESS"

except Exception, e:
  print "FAILURE"
  print str(e)

