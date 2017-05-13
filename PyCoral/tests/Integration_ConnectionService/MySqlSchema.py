class MySqlSchema:
 "A simple MySqlSchema class"
 m_proxy = 0
 def __init__( self, proxy ):
  print "MySqlSchema Constructor" 
  try:
   print "In MySqlSchema" , " Proxy = " , proxy
   self.m_proxy = proxy
  except Exception, e:
   raise Exception("Error in Initializer: " + str(e)) 

 def listTables(self, schema):
  "listTables method of MySqlSchema"
  try:
   print "In listTables"
   self.m_proxy.transaction().start()
   print "Tables in the schema: ", schema
   listOfTables = self.m_proxy.schema(schema).listTables()
   for iTable in listOfTables:
    print iTable
   self.m_proxy.transaction().commit()

   print "listTables SUCCESS"

  except Exception, e:
   raise Exception("Error in listTables method: " + str(e)) 

 def __del__( self ):
  print "MySqlSchema Destructor" 
