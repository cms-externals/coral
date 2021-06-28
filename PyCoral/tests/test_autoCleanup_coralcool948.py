import os, sys, time, getopt
os.environ['CORAL_MSGFORMAT']='ATLAS'
os.environ['CORAL_CONNECTIONPOOL_VERBOSE']='1'

from PyCoralTest import validateBackends
[urlRW,urlRO] = validateBackends( "oracle:oracle" )

def usage():
    print 'Usage:', sys.argv[0], '[-c <auto_cleanup (F|False|T|True)>] [-p <cleanup_period (seconds >= 0)>] [-t <connection_timeout (seconds >= 0)>]'
    sys.exit(1)

try:
    opts, args = getopt.getopt( sys.argv[1:], "hc:p:t:", ["help", "cleanup=", "period=","timeout="] )
except getopt.GetoptError:
    usage()

c=None
p=None
t=None
for opt, arg in opts:
    if opt in ("-h", "--help"):
        usage()
    elif opt in ("-c", "--cleanup"):
        if arg == 'T' or arg == 'True': c=True 
        elif arg == 'F' or arg == 'False': c=False
        else: usage()
    elif opt in ("-p", "--period"):
        try: p = int(arg) 
        except: usage()
        if p<0: usage()
    elif opt in ("-t", "--timeout"):
        try: t = int(arg) 
        except: usage()
        if t<0: usage()

if p is not None:
    print 'Set period to: ', p
    os.environ['CORAL_CONNECTIONPOOL_CLEANUPPERIOD']=str(p)
else:
    print 'Set period to:  DEFAULT'
    # NB: This must be set _before_ retrieving ConnectionService.configuration()
    os.environ['CORAL_CONNECTIONPOOL_CLEANUPPERIOD']='10' # easier...

import coral
svc=coral.ConnectionService()
cfg=svc.configuration()

if c is not None:
    print 'Set cleanup to:', c
    if c: cfg.enablePoolAutomaticCleanUp()
    else: cfg.disablePoolAutomaticCleanUp()
else:
    print 'Set cleanup to: DEFAULT'

if t is not None:
    print 'Set timeout to:', t
    cfg.setConnectionTimeOut(t)
else:
    print 'Set timeout to: DEFAULT'

print 'Timeout:', cfg.connectionTimeOut()
print 'Period: ', os.environ['CORAL_CONNECTIONPOOL_CLEANUPPERIOD']
print 'Cleanup:', cfg.isPoolAutomaticCleanUpEnabled()

###sys.exit(0)

# === WARNING #1: calling disablePoolAutomaticCleanUp() has no effect if the
# cleanup thread has already started, i.e. after the first call to connect()

# === WARNING #2: connection timeout and pool cleanup period are different;
# the latter is set by CORAL_CONNECTIONPOOL_CLEANUPPERIOD (CORALCOOL-847)

print
print '== Connect'
ses=svc.connect(urlRW)
print
print '== Disconnect'
ses=0
print
for i in range(0, 5):
    print '== Sleep 1 seconds (', i+1, 'of 5 )'
    time.sleep(1)

if not cfg.isPoolAutomaticCleanUpEnabled():
    print
    print '== Enable pool automatic cleanup'
    cfg.enablePoolAutomaticCleanUp()
else:
    print
    print '== Disable pool automatic cleanup (WARNING: NO EFFECT!)'
    cfg.disablePoolAutomaticCleanUp()

print
print '== Connect'
ses=svc.connect(urlRW)
print
print '== Disconnect'
ses=0
print
for i in range(0, 5):
    print '== Sleep 1 seconds (', i+1, 'of 5 )'
    time.sleep(1)

print
print '== Exit'

