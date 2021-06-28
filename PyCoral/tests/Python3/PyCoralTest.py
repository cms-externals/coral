#!/bin/env python
import os, sys, unittest

#-----------------------------------------------------------------------------

class TestCase( unittest.TestCase ):

    #------------------------------------------------------------------------

    def separator(self,char):
        return char*70

    #------------------------------------------------------------------------

    def __str__(self):
        # Modify the str representation of each test method in the TestCase
        # (used by _TextTestResult.getDescription in _TextTestResult.startTest)
        sep = self.separator("+")
        if "SLOTNAME" in os.environ and "BINARY_TAG" in os.environ: sep = "%s\n[%s on %s slot]"%(sep, os.environ["BINARY_TAG"], os.environ["SLOTNAME"])
        return "\n%s\n%s.%s"%(sep,self.__class__.__name__,self._testMethodName)

    #------------------------------------------------------------------------

    def tearDown(self):
        print(self.separator("="))

#-----------------------------------------------------------------------------

def buildUniqueTableName( name ):
    dbg = False
    # The unique prefix across nightly slots
    slotname = ""
    if "SLOTNAME" in os.environ:
        slotname = os.environ["SLOTNAME"]
    slotPrefix = ""
    if   slotname == "":
        slotPrefix = "N"
    elif slotname == "dev":
        slotPrefix = "A"
    elif slotname == "dev1":
        slotPrefix = "B"
    elif slotname == "dev2":
        slotPrefix = "C"
    elif slotname == "dev3":
        slotPrefix = "D"
    elif slotname == "dev4":
        slotPrefix = "E"
    elif slotname == "release":
        slotPrefix = "R"
    elif slotname == "test":
        slotPrefix = "T"
    elif slotname == "test1":
        slotPrefix = "U"
    elif slotname == "externals":
        slotPrefix = "X"
    else:
        slotPrefix = "Z"
    if dbg: print("Slot prefix:",slotPrefix)
    # The unique prefix across platforms
    platformPrefix = ""
    if "CORAL_BINARY_TAG_HASH" in os.environ:
        platformPrefix = os.environ["CORAL_BINARY_TAG_HASH"]
        if len(platformPrefix) != 3 :
            print("ERROR! CORAL_BINARY_TAG_HASH '"+platformPrefix+"' is not three-character long")
            exit(1)
        elif platformPrefix == '999' :
            print("ERROR! No platform-specific CORAL_BINARY_TAG_HASH is defined: add BINARY_TAG to CMakeLists.txt")
            exit(1)
    else:
        print("WARNING! CORAL_BINARY_TAG_HASH is not set")
    if dbg: print("Platform prefix:",platformPrefix)
    # Workaround for ORA-01466 (bug #87935)
    if 'CORAL_TESTSUITE_SLEEPFOR01466' in os.environ:
        os.environ[ 'CORAL_TESTSUITE_SLEEPFOR01466_PREFIX' ] = slotPrefix + platformPrefix
    # Return the unique name
    if name != "": return slotPrefix + platformPrefix + "_" + name
    else: return slotPrefix + platformPrefix

#-----------------------------------------------------------------------------

def buildUniquePortNumber():
    dbg = False
    # The unique number across nightly slots
    slotname = ""
    if "SLOTNAME" in os.environ:
        slotname = os.environ["SLOTNAME"]
    slotSuffix = ""
    if   slotname == "":
        slotSuffix = 0
    elif slotname == "dev":
        slotSuffix = 1
    elif slotname == "dev1":
        slotSuffix = 2
    elif slotname == "dev2":
        slotSuffix = 3
    elif slotname == "dev3":
        slotSuffix = 4
    elif slotname == "dev4":
        slotSuffix = 5
    elif slotname == "release":
        slotSuffix = 6
    elif slotname == "test":
        slotSuffix = 7
    elif slotname == "test1":
        slotSuffix = 8
    elif slotname == "externals":
        slotSuffix = 9
    else:
        slotSuffix = 0
    if dbg: print("Slot suffix:",slotSuffix)
    # The three-digit unique prefix across platforms
    platformSuffix = 0
    if "CORAL_BINARY_TAG_HASH" in os.environ:
        platformPrefix = os.environ["CORAL_BINARY_TAG_HASH"]
        if len(platformPrefix) != 3 :
            print("ERROR! CORAL_BINARY_TAG_HASH '"+platformPrefix+"' is not three-character long")
            exit(1)
        elif platformPrefix == '999' :
            print("ERROR! No platform-specific CORAL_BINARY_TAG_HASH is defined: add BINARY_TAG to CMakeLists.txt")
            exit(1)
        platformSuffix = int( platformPrefix )
    else:
        print("WARNING! CORAL_BINARY_TAG_HASH is not set")
    if dbg: print("Platform suffix:",platformSuffix)
    # Return the unique number
    return 40000 + slotSuffix*1000 + platformSuffix

#-----------------------------------------------------------------------------

def buildUrl( backend, readOnly ):
    if backend != "SQLite" :
        # Do not hardcode "sftnight" as the only alias user (CORALCOOL-2888)
        if "CORAL_QMTEST_USER" in os.environ:
            user=os.environ["CORAL_QMTEST_USER"]
        else:
            user = "sftnight"
        # Define read-only and read-write aliases for tests
        if readOnly:
            return "CORAL-" + backend + "-" + user + "/reader"
        else:
            return "CORAL-" + backend + "-" + user + "/admin"
    else:
        # Use unique SQLite database names across platforms/slots (bug #103484)
        return "sqlite_file:/tmp/" + os.environ["USER"] + "_" + buildUniqueTableName("") + ".db";

#-----------------------------------------------------------------------------

def validateBackends( arg ):
    if arg == "oracle" or arg == "oracle:oracle":
        urlRW = buildUrl( "Oracle", False )
        urlRO = buildUrl( "Oracle", True )
    elif arg == "oracle:frontier":
        urlRW = buildUrl( "Oracle", False )
        urlRO = buildUrl( "Frontier", True )
    elif arg == "oracle:coral":
        urlRW = buildUrl( "Oracle", False )
        urlRO = buildUrl( "CoralServer-Oracle", True )
    elif arg == "mysql" or arg == "mysql:mysql":
        urlRW = buildUrl( "MySQL", False )
        urlRO = buildUrl( "MySQL", True )
    elif arg == "mysql:coral":
        urlRW = buildUrl( "MySQL", False )
        urlRO = buildUrl( "CoralServer-MySQL", True )
    elif arg == "sqlite" or arg == "sqlite:sqlite":
        urlRW = buildUrl( "SQLite", False )
        urlRO = buildUrl( "SQLite", True )
    else:
        print("ERROR! Invalid backend pair argument '" + arg + "'")
        usage()
        exit(1)
    return [ urlRW, urlRO ]

#-----------------------------------------------------------------------------

def usage():
    print("Usage:", sys.argv[0], "writer:reader")
    print("Allowed values for writer:reader :\n" + \
          "  oracle:oracle\n" + \
          "  oracle:frontier\n" + \
          "  oracle:coral\n" + \
          "  mysql:mysql\n" + \
          "  mysql:coral\n" + \
          "  sqlite:sqlite")

#-----------------------------------------------------------------------------

def parseArguments():
    argc = len( sys.argv )
    if argc < 2 :
        print("ERROR! Missing backend argument writer:reader")
        usage()
        exit(1)
    arg = sys.argv[1]
    sys.argv.pop(1)
    return validateBackends( arg )

#-----------------------------------------------------------------------------

if __name__ == '__main__':
    uniqueTablePrefix = buildUniqueTableName( "" )
    print("Unique table prefix:", uniqueTablePrefix)
    uniquePortNumber = buildUniquePortNumber()
    print("Unique port number:", uniquePortNumber)
    urlsOF = validateBackends( "oracle:frontier" )
    print("URLs for 'oracle:frontier':", urlsOF)
