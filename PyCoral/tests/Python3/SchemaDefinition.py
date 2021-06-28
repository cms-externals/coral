import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time


class SchemaDefinition:
 m_connectionString = 0
 m_svc = 0
 "A SchemaDefinition class"
 def __init__( self, connectionString ):
  print("SchemaDefinition Constructor") 
  try:
   print("In SchemaDefinition" , " connectionString = " , connectionString) 
   self.m_connectionString = connectionString
   self.m_svc = coral.ConnectionService()
   print("In SchemaTest" , " m_connectionString = " , self.m_connectionString) 

  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def createSchema(self):
  "createSchema method of SchemaDefinition"
  try:
   print("In createSchema")
   m_proxy = self.m_svc.connect(self.m_connectionString, coral.access_Update)

   m_proxy.transaction().start()

   #Get rid of the previous tables and views
   m_proxy.nominalSchema().dropIfExistsView( "V0" )
   m_proxy.nominalSchema().dropIfExistsTable( "T3" )
   m_proxy.nominalSchema().dropIfExistsTable( "T2" )
   m_proxy.nominalSchema().dropIfExistsTable( "T1" )

   # Create the first table
   description1 = coral.TableDescription( "SchemaDefinition_Test" )
   description1.setName( "T1" )
   description1.insertColumn( "id","long" )
   # Define primary key
   description1.setPrimaryKey( "id" )
   description1.insertColumn( "t","short" )
   description1.insertColumn( "Xx","float" )
   # Set column not null
   description1.setNotNullConstraint( "Xx" )
   description1.insertColumn( "Y","double" )
   # Add a unique constraint specifying a name
   description1.setUniqueConstraint( "Y", "U_T1_Y" )
   description1.insertColumn( "Z","double" )
   # Add a variable size string
   description1.insertColumn( "Comment","string", 100, False )
   # Add a blob
   description1.insertColumn( "Data","blob" )

   # Create the actual table
   table = m_proxy.nominalSchema().createTable( description1 )


   # Change the column name
   table.schemaEditor().renameColumn( "Xx", "X" )

   # Change the column type
   table.schemaEditor().changeColumnType( "t","long long" )
   # Add a not null constaint
   table.schemaEditor().setNotNullConstraint( "t" )

   # Add a unique constraint made out of two columns
   constraintColumns = ("X","Z" )
   table.schemaEditor().setUniqueConstraint( constraintColumns )

   # Add a unique index
   table.schemaEditor().createIndex( "T1_IDX_t", "t", True )

   # Give privileges
   table.privilegeManager().grantToPublic( coral.privilege_Select )

   # Create the second table
   description2 = coral.TableDescription( "SchemaDefinition_Test" )
   description2.setName( "T2" )
   description2.insertColumn( "id","long" )
   description2.setUniqueConstraint( "id" )
   #description2.setPrimaryKey( "id" )
   description2.setNotNullConstraint( "id" )
   description2.insertColumn( "tx","float")
   description2.setNotNullConstraint( "tx" )
   description2.insertColumn( "ty","double" )
   description2.setNotNullConstraint( "ty" )
   m_proxy.nominalSchema().createTable( description2 ).privilegeManager().grantToPublic( coral.privilege_Select )



   # Create the third table
   description3 = coral.TableDescription( "SchemaDefinition_Test" )
   description3.setName( "T3" )
   description3.insertColumn( "id","long" )
   description3.setPrimaryKey( "id" )
   description3.insertColumn( "fk1","long" )
   description3.createForeignKey( "T3_FK1", "fk1", "T1", "id" )
   description3.insertColumn( "fk2","long" )
   description3.createForeignKey( "T3_FK2", "fk2", "T2", "id" )
   description3.insertColumn( "Qn","float" )
   m_proxy.nominalSchema().createTable( description3 ).privilegeManager().grantToPublic( coral.privilege_Select )

   # Create a view
   factory = m_proxy.nominalSchema().viewFactory()
   factory.addToOutputList( "T3.id", "id" )
   factory.addToOutputList( "T1.X", "x" )
   factory.addToOutputList( "T1.Y", "y" )
   factory.addToOutputList( "T1.Z", "z" )
   factory.addToOutputList( "T2.tx", "TX" )
   factory.addToOutputList( "T2.ty", "TY" )
   factory.addToTableList( "T3" )
   factory.addToTableList( "T1" )
   factory.addToTableList( "T2" )
   factory.setCondition( "T1.id = T3.fk1 AND T2.id = T3.fk2 AND T3.Qn > 0.5", coral.AttributeList())
   factory.create( "V0" ).privilegeManager().grantToPublic( coral.privilege_Select )
   del factory

   m_proxy.transaction().commit()
   del m_proxy 
   print("createSchema SUCCESS")
   time.sleep(1)
   return True

  except Exception as e:
   raise Exception("Error in createSchema method: " + str(e)) 
   return False

 def printViewInfo(self, view ):
  "printViewInfo method of SchemaDefinition"
  try:
   print("In printViewInfo")
   numberOfColumns = view.numberOfColumns()
   print("View " , view.name())
   print(" has", "  ", numberOfColumns , " columns :")
   for i in range( 0,numberOfColumns ):
    column = view.column( i )
    print("    " , column.name(), " (", column.type() , ")")
    if ( column.isUnique() ):
     print(" UNIQUE")
    if ( column.isNotNull() ):
     print(" NOT NULL")

   print("  definition string : " , view.definition())

  except Exception as e:
   raise Exception("Error in printViewInfo method: " + str(e))


 def printTableInfo(self, description ):
  "printTableInfo method of SchemaTest"
  try:
   print("In printTableInfo")

   print(description)
   numberOfColumns = description.numberOfColumns()
   print("Table " , description.name())
   tableSpaceName = description.tableSpaceName()
   if ( not tableSpaceName ):
     print(" (created in tablespace " , tableSpaceName , ")")
   print(" has", end=' ')
   print("  " , numberOfColumns , " columns :") 
   for  i in range(0, numberOfColumns):
     column = description.columnDescription( i )
     print("    " , column.name() , " (" , column.type() , ")")
     if ( column.isUnique() ): 
      print(" UNIQUE");
     if ( column.isNotNull() ):
      print(" NOT NULL")

   if ( description.hasPrimaryKey() ):
     columnNames = description.primaryKey().columnNames()
     print("  Primary key defined for column")
     if ( len(columnNames) > 1 ):
      print("s")
     print(" ")
     for iColumn in columnNames:
      print(iColumn , " ")

     tableSpace = description.primaryKey().tableSpaceName()
     if ( not tableSpace ):
      print(" in tablespace " , tableSpace)

   numberOfUniqueConstraints = description.numberOfUniqueConstraints()
   print("  " , numberOfUniqueConstraints)
   if ( numberOfUniqueConstraints == 1 ):
    print(" unique constraint:")
   else:
    if ( numberOfUniqueConstraints == 0 ):
     print(" unique constraints")
    else:
     print(" unique constraints:")

   for i in range( 0, numberOfUniqueConstraints ):
     uniqueConstraint = description.uniqueConstraint( i )
     print("    " , uniqueConstraint.name() , " defined for column")
     columnNames = uniqueConstraint.columnNames()
     if ( len(columnNames) > 1 ):
      print("s")
     print(" ")
     for iColumn in columnNames:
       print(iColumn)

     tableSpace = uniqueConstraint.tableSpaceName()
     if ( not tableSpace ):
       print(" in tablespace ", tableSpace)

   numberOfIndices = description.numberOfIndices()
   print("  " , numberOfIndices)
   if ( numberOfIndices == 1 ):
    print(" index:") 
   else:
    if ( numberOfIndices == 0 ):
     print(" indices")
    else:
     print(" indices:")
   for i in range(0, numberOfIndices ):
     index = description.index( i )
     print("    " , index.name())
     if ( index.isUnique() ):
      print(" (UNIQUE)")
     print(" defined for column")
     columnNames = index.columnNames()
     if ( len(columnNames) > 1 ):
      print("s")
     print(" ");
     for iColumn in columnNames:
       print(iColumn)

     tableSpace = index.tableSpaceName()
     if ( not tableSpace ):
       print(" in tablespace " , tableSpace);

   numberOfForeignKeys = description.numberOfForeignKeys()
   print("  " , numberOfForeignKeys)
   if ( numberOfForeignKeys == 1 ):
    print(" foreign key:")
   else:
    if ( numberOfForeignKeys == 0 ):
     print(" foreign keys") 
    else:
     print(" foreign keys:")
   for i in range(0, numberOfForeignKeys):
     foreignKey = description.foreignKey( i )
     print("    " , foreignKey.name() , " defined for column")
     columnNames = foreignKey.columnNames()
     if ( len(columnNames) > 1 ):
      print("s")
     print(" ")

     for iColumn in columnNames:
       print(iColumn)

     print(" -> " , foreignKey.referencedTableName() , "( "); 
     columnNamesR = foreignKey.referencedColumnNames()
     for iColumn in columnNamesR:
       print(iColumn)

     print(" )")


  except Exception as e:
   raise Exception("Error in printTableInfo method: " + str(e))



 def readSchema(self ):
  "readSchema method of SchemaDefinition"
  try:
   print("In readSchema")

   m_proxy = self.m_svc.connect(self.m_connectionString, coral.access_ReadOnly)

   m_proxy.transaction().start( True )

   # Examine the tables
   self.printTableInfo( m_proxy.nominalSchema().tableHandle( "T1" ).description() )
   self.printTableInfo( m_proxy.nominalSchema().tableHandle( "T2" ).description() )
   self.printTableInfo( m_proxy.nominalSchema().tableHandle( "T3" ).description() )
   self.printViewInfo( m_proxy.nominalSchema().viewHandle( "V0" ) )
 
   m_proxy.transaction().commit()

   del m_proxy
   print("readSchema SUCCESS")
   return True

  except Exception as e:
   print("Error in readSchema method: " . str(e))
   return False
   

 def __del__( self ):
  print("Schema Definiton Destructor") 
