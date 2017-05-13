import os
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import time
import math
from threading import Thread


class ThreadBody(Thread):
 "ThreadBody class"
 def __init__( self, connection, threadID, schemaName, userName, password ):
  Thread.__init__(self)
  m_connection = 0
  m_threadID = 0
  m_schemaName = 0
  m_userName = 0
  m_password = 0
  print "ThreadBody Constructor" 
  try:
   print "In ThreadBody" , " connection = " , connection 
   print "In ThreadBody" , " threadID = " , threadID 
   print "In ThreadBody" , " schemaName = " , schemaName 
   print "In ThreadBody" , " userName = " , userName 
   print "In ThreadBody" , " password = " , password 
   self.m_connection = connection
   self.m_threadID = threadID
   self.m_schemaName = schemaName
   self.m_userName = userName
   self.m_password = password

  except Exception, e:
   raise Exception("Error in Initializer: " + str(e)) 

 def run(self):
  "Run method of ThreadBody"
  try:
    print "In ThreadBody: run"
    print "Thread no " , self.m_threadID , " running..."

    session = self.m_connection
    session.transaction().start()
    schema = self.m_schemaName

    print "Thread no ", self.m_threadID ," about to create and fill a table"

    #The table name
    tableName = "ORA_UT_MS_T_";
    print tableName,self.m_threadID

    schema.dropIfExistsTable( tableName )
    description = coral.TableDescription()
    description.setName( tableName )
    description.insertColumn( "I", "int" )
    description.insertColumn( "F", "float" )
    description.insertColumn( "D", "double" )
    description.setPrimaryKey( "I" )
    description.setNotNullConstraint( "F" )

    table = schema.createTable( description )
    rowBuffer = coral.AttributeList()
    rowBuffer.extend( "I","int" )
    rowBuffer.extend( "F","float" )
    rowBuffer.extend( "D","double" )

    for row in range( 0, 100 ):
      i = row;
      rowBuffer[0].setData(i)
      f = row + 0.001 * self.m_threadID;
      rowBuffer[1].setData(f)
      d = row + 0.000001 * self.m_threadID;
      rowBuffer[2].setData(d)
      table.dataEditor().insertRow( rowBuffer )

    session.transaction().commit()
    time.sleep(1)

    session.transaction().start( True )
    print "Thread no " ,self.m_threadID , " about to perform a query"

    query = schema.tableHandle( tableName ).newQuery()

    outputBuffer = coral.AttributeList()
    outputBuffer.extend( "RES","double" )

    query.addToOutputList( "F+D", "RES" )
    query.addToOrderList( "I" )
    query.defineOutput( outputBuffer )

    cursor = query.execute()
    row = 0
    #res = outputBuffer[0].data()
    while ( cursor.next() ):
      res = outputBuffer[0].data()
      if ( math.fabs( res - ( 2* row + 0.001001 * self.m_threadID ) ) > 0.00001 ):
        raise RuntimeError( "Unexpected data" )
      row = row + 1

    if ( row != 100 ):
      raise RuntimeError( "Unexpected number of rows" )

    session.transaction().commit()

  except Exception, e:
   raise Exception("Error in Run method: " + str(e)) 

 def __del__( self ):
  print "ThreadBody Destructor" 
