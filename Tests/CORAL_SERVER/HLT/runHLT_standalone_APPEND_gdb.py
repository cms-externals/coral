##############################################################
# Add the following lines to runHLT_standalone.py
# and save the result as runHLT_standalone_gdb.py
##############################################################

# See https://twiki.cern.ch/twiki/bin/view/Atlas/StartingDebuggerWithAthenaPy
theApp.getHandle().Dlls += ['IOVDbSvc'] # i.e. libIOVDbSvc.so
from AthenaCommon.Debugging import *
###help(hookDebugger)
hookDebugger()

# Interactive gdb session
###(gdb) sha IOVDbSvc
###(gdb) info signals
###(gdb) dir /afs/cern.ch/atlas/software/builds/AtlasCore/15.2.0/Database/IOVDbSvc/src
###(gdb) br 'IOVDbFolder::loadCache(unsigned long long, unsigned int, std::string const&, bool)' 
###(gdb) br IOVDbFolder.cxx:356
###(gdb) display dpPtr
###(gdb) c

###import time
###time.sleep(10)
