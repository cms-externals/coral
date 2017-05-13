import TestDriver
import coral

try:
 for i in ( 0,2 ):
  os = i+1;
  serviceName = str(os)
  print  "    preparing the schema in " , serviceName , "..."
  TestDriver.prepareSchema( serviceName )
  print "    simple row inserts in " , serviceName , "..."
  TestDriver.writeSimple( serviceName )
  print "    bulk row inserts in ", serviceName , "..."
  TestDriver.writeBulk( serviceName )
  print "    reading back the data from " , serviceName , "..."
  TestDriver.read( serviceName )
  print "[SUCCESS] Tests for database service " , serviceName ," passed." 

except Exception, e:
 print "Service FAILURE"
 print str(e)
