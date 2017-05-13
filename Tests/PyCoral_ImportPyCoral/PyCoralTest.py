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
        return "\n%s\n%s.%s"%(sep,self.__class__.__name__,self._testMethodName)

    #------------------------------------------------------------------------

    def tearDown(self):
        print self.separator("=")

#-----------------------------------------------------------------------------

def buildUniqueTableName( name ):
    dbg = False
    # The unique prefix across nightly slots
    slotname = ""
    if "LCG_NGT_SLT_NAME" in os.environ:
        slotname = os.environ["LCG_NGT_SLT_NAME"]
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
    elif slotname == "test2":
        slotPrefix = "V"
    else:
        slotPrefix = "Z"
    if dbg: print "Slot prefix:",slotPrefix
    # The unique prefix across platforms
    cmtconfig = ""
    if "CMTCONFIG" in os.environ:
        cmtconfig = os.environ["CMTCONFIG"]
    platformPrefix = ""
    if cmtconfig == "":
        platformPrefix = "NIL"
    elif cmtconfig == "slc4_ia32_gcc34":
        platformPrefix = "4TO"
    elif cmtconfig == "slc4_ia32_gcc34_dbg":
        platformPrefix = "4TD"
    elif cmtconfig == "slc4_amd64_gcc34":
        platformPrefix = "4SO"
    elif cmtconfig == "slc4_amd64_gcc34_dbg":
        platformPrefix = "4SD"
    elif cmtconfig == "i686-slc5-gcc43-opt":
        platformPrefix = "5TO"
    elif cmtconfig == "i686-slc5-gcc43-dbg":
        platformPrefix = "5TD"
    elif cmtconfig == "i686-slc5-gcc43-cov":
        platformPrefix = "5TC"
    elif cmtconfig == "x86_64-slc5-gcc43-opt":
        platformPrefix = "5SO"
    elif cmtconfig == "x86_64-slc5-gcc43-dbg":
        platformPrefix = "5SD"
    elif cmtconfig == "x86_64-slc5-gcc43-cov":
        platformPrefix = "5SC"
    elif cmtconfig == "x86_64-slc5-llvm27-dbg":
        platformPrefix = "5SL"
    elif cmtconfig == "x86_64-slc5-gccmax-dbg":
        platformPrefix = "5SM"
    elif cmtconfig == "x86_64-slc5-icc11-dbg":
        platformPrefix = "5SI"
    elif cmtconfig == "i386-mac106-gcc42-opt":
        platformPrefix = "MTO"
    elif cmtconfig == "i386-mac106-gcc42-dbg":
        platformPrefix = "MTD"
    elif cmtconfig == "x86_64-mac106-gcc42-opt":
        platformPrefix = "MSO"
    elif cmtconfig == "x86_64-mac106-gcc42-dbg":
        platformPrefix = "MSD"
    elif cmtconfig == "win32_vc71_dbg":
        platformPrefix = "VC7"
    elif cmtconfig == "i686-winxp-vc9-dbg":
        platformPrefix = "VC9"
    else:
        platformPrefix = "000"
    if dbg: print "Platform prefix:",platformPrefix
    # Return the unique name
    return slotPrefix + platformPrefix + "_" + name

#-----------------------------------------------------------------------------

def buildUniquePortNumber():
    dbg = False
    # The unique number across nightly slots
    slotname = ""
    if "LCG_NGT_SLT_NAME" in os.environ:
        slotname = os.environ["LCG_NGT_SLT_NAME"]
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
    elif slotname == "test2":
        slotSuffix = 9
    else:
        slotSuffix = 0
    if dbg: print "Slot suffix:",slotSuffix
    # The unique prefix across platforms
    cmtconfig = ""
    if "CMTCONFIG" in os.environ:
        cmtconfig = os.environ["CMTCONFIG"]
    platformSuffix = 0
    if cmtconfig == "":
        platformSuffix = 900
    elif cmtconfig == "slc4_ia32_gcc34":
        platformSuffix = 430
    elif cmtconfig == "slc4_ia32_gcc34_dbg":
        platformSuffix = 431
    elif cmtconfig == "slc4_amd64_gcc34":
        platformSuffix = 460
    elif cmtconfig == "slc4_amd64_gcc34_dbg":
        platformSuffix = 461
    elif cmtconfig == "i686-slc5-gcc43-opt":
        platformSuffix = 530
    elif cmtconfig == "i686-slc5-gcc43-dbg":
        platformSuffix = 531
    elif cmtconfig == "i686-slc5-gcc43-cov":
        platformSuffix = 532
    elif cmtconfig == "x86_64-slc5-gcc43-opt":
        platformSuffix = 560
    elif cmtconfig == "x86_64-slc5-gcc43-dbg":
        platformSuffix = 561
    elif cmtconfig == "x86_64-slc5-gcc43-cov":
        platformSuffix = 562
    elif cmtconfig == "x86_64-slc5-llvm27-dbg":
        platformSuffix = 563
    elif cmtconfig == "x86_64-slc5-gccmax-dbg":
        platformSuffix = 564
    elif cmtconfig == "x86_64-slc5-icc11-dbg":
        platformSuffix = 565
    elif cmtconfig == "i386-mac106-gcc42-opt":
        platformSuffix = 830
    elif cmtconfig == "i386-mac106-gcc42-dbg":
        platformSuffix = 831
    elif cmtconfig == "x86_64-mac106-gcc42-opt":
        platformSuffix = 860
    elif cmtconfig == "x86_64-mac106-gcc42-dbg":
        platformSuffix = 861
    elif cmtconfig == "win32_vc71_dbg":
        platformSuffix = 7
    elif cmtconfig == "i686-winxp-vc9-dbg":
        platformSuffix = 9
    else:
        platformSuffix = 999
    if dbg: print "Platform suffix:",platformSuffix
    # Return the unique number
    return 40000 + slotSuffix*1000 + platformSuffix

#-----------------------------------------------------------------------------

def buildUrl( backend, readOnly ):
    if readOnly:
        return "CORAL-" + backend + "-lcgnight/reader"
    else:
        return "CORAL-" + backend + "-lcgnight/admin"

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
        print "ERROR! Invalid backend pair argument '" + arg + "'"
        usage()
        exit(1)
    return [ urlRW, urlRO ]

#-----------------------------------------------------------------------------

def usage():
    print "Usage:", sys.argv[0], "writer:reader"
    print "Allowed values for writer:reader :\n" + \
          "  oracle:oracle\n" + \
          "  oracle:frontier\n" + \
          "  oracle:coral\n" + \
          "  mysql:mysql\n" + \
          "  mysql:coral\n" + \
          "  sqlite:sqlite"
    
#-----------------------------------------------------------------------------

def parseArguments():
    argc = len( sys.argv ) 
    if argc < 2 :
        print "ERROR! Missing backend argument writer:reader"
        usage()
        exit(1)
    arg = sys.argv[1]
    sys.argv.pop(1)
    return validateBackends( arg )

#-----------------------------------------------------------------------------

if __name__ == '__main__':    
    uniqueTablePrefix = buildUniqueTableName( "" )
    print "Unique table prefix:", uniqueTablePrefix
    uniquePortNumber = buildUniquePortNumber()
    print "Unique port number:", uniquePortNumber
    urlsOF = validateBackends( "oracle:frontier" )
    print "URLs for 'oracle:frontier':", urlsOF
