import MultipleSessionsApp
import coral
import os


try: 
 #Set the authentication path
 if ( not os.getenv( "CORAL_AUTH_PATH" ) ):
  os.putenv( "CORAL_AUTH_PATH", "/afs/cern.ch/sw/lcg/app/pool/db" )

 app = MultipleSessionsApp.MultipleSessionsApp( "oracle://lcg_coral_nightly/lcg_coral_nightly",
                        "lcg_coral_nightly",
                        "xxxx")
 app.run()
 print "Overall SUCCESS"
except Exception, e:
 print "Overall FAILURE"
 print str(e)

