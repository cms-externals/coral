import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import SchemaDefinition
import DmlOperations
import Queries
import InsertSelect
import SetOperations


class TestApp:
 "A Basic Test for Integration"
 def __init__( self, writerConnectionString, readerConnectionString, readerWorkingSchema ):
  m_writerConnectionString = 0
  m_readerConnectionString = 0
  m_readerWorkingSchema = 0
  print("TestApp Constructor") 
  try:
   print("In TestApp" , " WriterConnectionString = " , writerConnectionString) 
   print("In TestApp" , " ReaderConnectionString = " , readerConnectionString) 
   print("In TestApp" , " ReaderWorkingSchema = " , readerWorkingSchema) 
   self.m_writerConnectionString = writerConnectionString
   self.m_readerConnectionString = readerConnectionString
   self.m_readerWorkingSchema = readerWorkingSchema
   #self.loadServices()
  except Exception as e:
   raise Exception("Error in Initializer: " + str(e)) 

 def loadServices(self):
  "loadServices method of TestApp"
  try:
   print("In TestApp: loadServices")
  except Exception as e:
   raise Exception("Error in Run method: " + str(e)) 

 def run(self):
  "Run method of TestApp"
  try:
   print("In TestApp: run")
   print("Test 1: defining and reading back tables and views")
   test1 = SchemaDefinition.SchemaDefinition(self.m_writerConnectionString )
   result = (test1.createSchema() and test1.readSchema())
   if ( not result ):
    raise Exception( "Test failed" )

   print("Test 2: DML operations")
   test2 = DmlOperations.DmlOperations( self.m_writerConnectionString )
   result = (test2.atomicOperations() and test2.bulkOperations())
   del test2
   if ( not result ):
    raise Exception( "Test failed" )

   print("Test 3: Queries")
   test3 = Queries.Queries( self.m_readerConnectionString, self.m_readerWorkingSchema )
   result = test3.run()
   del test3
   if ( not result ):
    raise Exception( "Test failed" )

   print("Test 4: Insert/Select statements")
   test4 = InsertSelect.InsertSelect( self.m_writerConnectionString )
   result = test4.run()
   del test4
   if ( not result ):
    raise Exception( "Test failed" )

   print("Test 5: Queries involving set operations")
   test5 = SetOperations.SetOperations( self.m_readerConnectionString, self.m_readerWorkingSchema )
   result = test5.run()
   del test5
   if ( not result ):
    raise Exception( "Test failed" )

  except Exception as e:
   raise Exception("Error in Run method: " + str(e)) 

 def __del__( self ):
  print("TestApp Destructor") 
