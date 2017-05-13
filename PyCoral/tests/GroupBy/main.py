import GroupApp
import coral
import os
import sys

try: 
 #Set the authentication path
 app = GroupApp.GroupApp( "/dbdev/service_1",
                        "ral_writer",
                        "ral" )
 #startRef = sys.gettotalrefcount()
 for i in range(1,2):  
  app.run()
  print "Overall SUCCESS"
 del app
 #print "StartRef = ", startRef 
 #del startRef
 del i
 #print "EndRef = ", sys.gettotalrefcount() 
except Exception, e:
 print "Overall FAILURE"
 print str(e)

