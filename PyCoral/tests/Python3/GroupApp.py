import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time


class GroupApp:
 m_connectionString = 0
 m_svc = 0
 m_userName = 0
 m_password = 0
 "A SchemaDefinition class"
 def __init__( self, connectionString, userName, password ):
  print("SchemaDefinition Constructor") 
  try:
   print("In GroupApp" , " connectionString = " , connectionString) 
   self.m_connectionString = connectionString
   self.m_userName = userName 
   self.m_password = password 
   self.m_svc = coral.ConnectionService()
   print("In GroupApp" , " m_connectionString = " , self.m_connectionString) 
   #print "In GroupApp" , " m_userName = " , self.m_userName 
   #print "In GroupApp" , " m_password = " , self.m_password 

  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def fillData(self):
  "fillData method of GroupApp"
  try:
   print("In fillData")
   #m_proxy = self.m_svc.connect(self.m_connectionString, self.m_userName, self.m_password)
   m_proxy = self.m_svc.connect(self.m_connectionString, coral.access_Update)

   m_proxy.transaction().start()
   schema = m_proxy.nominalSchema()

   #Get rid of the previous tables and views
   schema.dropIfExistsTable( "ORA_UT_GB_T" )

   print("Describing new table")
   description = coral.TableDescription()
   description.setName( "ORA_UT_GB_T" )
   description.insertColumn( "ID", "int" )
   description.setPrimaryKey( "ID" )
   description.insertColumn( "x", "float" )
   description.setNotNullConstraint( "x" )
   description.insertColumn( "GrB","int")

   print("About to create the table")
   table = schema.createTable( description )

   rowBuffer = coral.AttributeList()
   rowBuffer.extend( "ID","int" )
   rowBuffer.extend( "x","float" )
   rowBuffer.extend( "GrB","int" )

   for i in range( 0,100 ):
    rowBuffer["ID"].setData(i)
    rowBuffer["GrB"].setData(i%10)
    rowBuffer["x"].setData(( i%14 ) + 0.1 * ( i%17 ))
    table.dataEditor().insertRow( rowBuffer );

   m_proxy.transaction().commit()
   del m_proxy
   print("fillData SUCCESS")
   return True

  except Exception as e:
   raise Exception("Error in fillData method: " + str(e)) 
   return False

 def readData(self ):
  "readData method of GroupBy"
  try:
   print("In readData")
   #m_proxy = self.m_svc.connect(self.m_connectionString, self.m_userName, self.m_password)
   m_proxy = self.m_svc.connect(self.m_connectionString, coral.access_ReadOnly)

   m_proxy.transaction().start( True )

   schema = m_proxy.nominalSchema()

   query = schema.tableHandle("ORA_UT_GB_T").newQuery()

   rowBuffer = coral.AttributeList()
   rowBuffer.extend( "GrB","int" )
   rowBuffer.extend( "RES","float" )

   query.addToOutputList( "GrB" )
   query.addToOrderList( "GrB" )
   query.addToOutputList( "AVG(x)", "RES" )
   query.groupBy( "GrB" )

   query.defineOutput( rowBuffer )

   cursor = query.execute()

   i = 0
   while ( cursor.next() ):
    print(str(rowBuffer))
    if ( i != rowBuffer["GrB"].data() ):
      raise RuntimeError( "Unexpected data" )
    i = i + 1

   if ( i != 10 ):
    raise RuntimeError( "Unexpected number of rows" ) 

   del query

   m_proxy.transaction().commit()
   del m_proxy

   print("readData SUCCESS")
   return True

  except Exception as e:
   print("Error in readData method: " . str(e))
   return False
   
 def run(self):
  try:
   print("GroupBy: In Run")
   self.fillData()
   time.sleep(1)
   self.readData()

  except Exception as e:
   raise Exception("Error in Run method: " + str(e))

 def __del__( self ):
  print("GroupBy Destructor") 
