releaseTag = 'CORAL_1_8_1'
#releaseTag = 'CORAL-preview'

cmtLCGconfig = 'LCGCMT_52'
#cmtLCGconfig = 'LCGCMT_preview'

packages=[]
packages.append( ('CoralBase', '1-7-1') ) 
packages.append( ('RelationalAccess', '1-17-0') )
packages.append( ('CoralCommon', '1-4-1') )
packages.append( ('PyCoral', '1-1-0') )
packages.append( ('ConnectionService', '2-7-2') )
packages.append( ('RelationalService', '1-1-0') )
packages.append( ('MonitoringService', '1-0-0') )
packages.append( ('XMLLookupService', '1-1-0') )
packages.append( ('LFCLookupService', '1-1-0') )
packages.append( ('LFCReplicaService', '1-0-0') )
packages.append( ('EnvironmentAuthenticationService', '1-2-0') )
packages.append( ('XMLAuthenticationService', '1-2-0') )
packages.append( ('OracleAccess', '1-6-0') )
packages.append( ('SQLiteAccess', '1-5-1') )
packages.append( ('MySQLAccess', '1-6-1') )
packages.append( ('FrontierAccess', '1-7-1') )

publicPackages = ( 'CoralBase', 'RelationalAccess' )

pythonModules = ( 'coral', )

testLibraries=[]
testLibraries.append( ('Connectivity', '1-0-1' ) )

tests=[]
tests.append( ('Integration_Basic', '1-2-0' ) )
tests.append( ('Integration_InputOutput', '1-1-1' ) )
tests.append( ('Integration_Monitoring', '1-3-0' ) )
tests.append( ('Integration_MonitoringService', '1-1-0') )
tests.append( ('Integration_ConnectionService', '1-2-0' ) )
tests.append( ('Integration_ConnectionService_CleanUpPool', '1-1-0' ) )
tests.append( ('Integration_ConnectionService_ConnectionPool', '1-0-0' ) )
#tests.append( ('Integration_ConnectionService_ConnectionPool_LFCLookupService', '0-0-2' ) )
#tests.append( ('Integration_ConnectionService_LFCLookupService', '0-0-2' ) )
tests.append( ('Integration_ConnectionService_ReplicaHandling', '0-1-0' ) )
tests.append( ('Integration_ConnectionService_ReplicaExcluding', '0-0-1' ) )
tests.append( ('Integration_ConnectionService_ReplicaSorting', '0-0-1' ) )
#tests.append( ('Integration_ConnectionService_ReplicaHandling_LFCLookupService', '0-0-2' ) )
tests.append( ('Integration_ConnectionService_SQLiteFailover', '0-1-0' ) )
tests.append( ('Integration_ConnectionService_SQLiteConnection', '0-0-1' ) )
tests.append( ('Integration_ConnectionService_SessionSharing', '0-1-0' ) )
#tests.append( ('Integration_ConnectionService_SimpleFailover', '0-1-0' ) )
#tests.append( ('Integration_ConnectionService_SimpleFailover_LFCLookupService', '0-0-2' ) )
tests.append( ('Integration_ConnectionService_SwitchSchema', '0-1-0' ) )
tests.append( ('Integration_ConnectionService_TimerBasic', '0-1-0' ) )
tests.append( ('Integration_StoredProcedures', '0-0-1' ) )
tests.append( ('System_SessionsInMultipleThreads', '0-1-0' ) )
