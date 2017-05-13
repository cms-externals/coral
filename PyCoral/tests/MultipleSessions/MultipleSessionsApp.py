import os
import time
os.environ['CORAL_DBLOOKUP_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
os.environ['CORAL_AUTH_PATH']='/afs/cern.ch/sw/lcg/app/pool/db'
import coral
import ThreadBody


class MultipleSessionsApp:
 "A Basic MultipleSessions test"
 def __init__( self, connectionString, userName, password ):
  m_connectionString = 0
  m_userName = 0
  m_password = 0
  print "MultipleSessionsApp Constructor" 
  try:
   print "In MultipleSessionsApp" , " ConnectionString = " , connectionString 
   print "In MultipleSessionsApp" , " userName = " , userName 
   print "In MultipleSessionsApp" , " password = " , password 
   self.m_connectionString = connectionString
   self.m_userName = userName
   self.m_password = password
   #self.loadServices()
  except Exception, e:
   raise Exception("Error in Initializer: " + str(e)) 

 def loadServices(self):
  "loadServices method of MultipleSessionsApp"
  try:
   print "In MultipleSessionsApp: loadServices"
  except Exception, e:
   raise Exception("Error in Run method: " + str(e)) 

 def run(self):
  "Run method of MultipleSessionsApp"
  try:
   print "In MultipleSessionsApp: run"
   print "MultipleSessions 1: Creating ISessionProxy and Schema objects"
   svc = coral.ConnectionService()
   session1 = svc.connect(self.m_connectionString, accessMode = coral.access_Update)
   schema = session1.nominalSchema()
   print time.ctime()
   threadBodies = []
   for i in range(0,1):
    current = ThreadBody.ThreadBody(session1,i,schema,self.m_userName,self.m_password)
    threadBodies.append(current)
    current.start()
   print "Waiting for the Threads....."
   
   for threadBody in threadBodies: 
    threadBody.join()
    del threadBody
   print time.ctime()

   print time.ctime()

  except Exception, e:
   raise Exception("Error in Run method: " + str(e)) 

 def __del__( self ):
  print "MultipleSessionsApp Destructor" 
