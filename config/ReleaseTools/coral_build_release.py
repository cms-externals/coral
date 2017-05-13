#!/usr/bin/env python

import os
import os.path
import sys
import string
import shutil
import time

import coral_config

cmtVersion = 'v1r18p20061003'
cmtReleaseDirectory='/afs/cern.ch/sw/contrib/CMT'
wincmtReleaseDirectory='z:\\cern.ch\\sw\\contrib\\CMT'

lcgcmtConfigParentDirectory = '/afs/cern.ch/sw/lcg/app/releases/LCGCMT'
winlcgcmtConfigParentDirectory = 'z:\\cern.ch\\sw\\lcg\\app\\releases\\LCGCMT'

logDir = ''

cmtsetup = ''

def setupEnvironment( release_directory, platform ):
    cmtpathforlcg = ''
    tcmtsetup = ''
    if string.find( platform, 'win32_vc' ) >= 0:
        tcmtsetup = 'set SITEROOT=z:\\cern.ch& '
        cmtpathforlcg = os.path.join( release_directory, 'src' ) + os.pathsep + os.path.join( winlcgcmtConfigParentDirectory, coral_config.cmtLCGconfig )
        tcmtsetup = tcmtsetup + os.path.join( wincmtReleaseDirectory, cmtVersion, 'mgr', 'setup.bat' ) + '& set CMTCONFIG=' + platform + '& '
        tcmtsetup = tcmtsetup + 'set CMTPATH=' + cmtpathforlcg + '& '
    else:
        tcmtsetup = 'export SITEROOT=/afs/cern.ch; '
        cmtpathforlcg = os.path.join( release_directory, 'src' ) + os.pathsep + os.path.join( lcgcmtConfigParentDirectory, coral_config.cmtLCGconfig )
        tcmtsetup = tcmtsetup + 'source ' + os.path.join( cmtReleaseDirectory, cmtVersion, 'mgr', 'setup.sh' ) + '; export CMTCONFIG=' + platform + '; '
        tcmtsetup = tcmtsetup + 'export CMTPATH=' + cmtpathforlcg + '; '
    return tcmtsetup


def removeTemporaries( platform ):
    if os.path.exists( platform ): shutil.rmtree( platform )
    if os.path.exists( os.path.join( 'Tests', platform ) ): shutil.rmtree( os.path.join( 'Tests', platform ) )
    if os.path.exists( os.path.join( 'config', platform ) ): shutil.rmtree( os.path.join( 'config', platform ) )
    for package in coral_config.packages:
        if os.path.exists( os.path.join( package[0], platform ) ): shutil.rmtree( os.path.join( package[0], platform ) )
    return


def buildPackage( package, platform ):
    if os.path.exists( os.path.join( package, 'cmt' ) ) and os.path.exists( os.path.join( package, 'src' ) ) :
        print 'Building library ' + package
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( package, 'cmt' ) )
        ret = os.system( cmtsetup + 'cmt make' )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, package + '.' + platform + '.err' ), 'a' )
            logFile.write( 'LIBRARY : \nERROR BUILDING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    if os.path.exists( os.path.join( package, 'cmt' ) ) and os.path.exists( os.path.join( package, 'tests' ) ) :
        print 'Building unit tests for ' + package
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( package, 'cmt' ) )
        ret = os.system( cmtsetup + 'cmt make tests' )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, package + '.' + platform + '.err' ), 'a' )
            logFile.write( 'UNIT TESTS : \nERROR BUILDING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    if os.path.exists( os.path.join( package, 'cmt' ) ) and os.path.exists( os.path.join( package, 'utilities' ) ) :
        print 'Building the utilities ' + package
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( package, 'cmt' ) )
        ret = os.system( cmtsetup + 'cmt make utilities' )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, package + '.' + platform + '.err' ), 'a' )
            logFile.write( 'UTILITIES : \nERROR BUILDING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    return



def buildTestLibrary( testLibrary, platform ):
    if os.path.exists( os.path.join( 'Tests', 'Libraries', testLibrary, 'src' ) ) and os.path.exists( os.path.join( 'Tests', 'cmt' ) ) :
        print 'Building test library ' + testLibrary
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( 'Tests', 'cmt' ) )
        ret = os.system( cmtsetup + 'cmt make test_' + testLibrary )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, 'Test-Libraries-' + testLibrary + '.' + platform + '.err' ), 'a' )
            logFile.write( 'TEST LIBRARY : \nERROR BUILDING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    return



def buildTest( test, platform ):
    if os.path.exists( os.path.join( 'Tests', test, 'src' ) ) and os.path.exists( os.path.join( 'Tests', 'cmt' ) ) :
        print 'Building test ' + test
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( 'Tests', 'cmt' ) )
        ret = os.system( cmtsetup + 'cmt make test_' + test )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, 'Test-' + test + '.' + platform + '.err' ), 'a' )
            logFile.write( 'TEST : \nERROR BUILDING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    return



def refreshPlugins( release_directory, platform ):
    if os.path.exists( os.path.join( 'config', 'cmt' ) ):
        print 'Refreshing the plugin cache'
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( 'config', 'cmt' ) )
        cmd = ''
        if string.find( platform, 'win32_vc' ) >= 0 :
            cmd = cmtsetup + 'call setup.bat & SealPluginDump'
        else:
            cmd = cmtsetup + '. ./setup.sh; SealPluginDump'
        ret = os.system( cmd )
        if ret != 0:
            # Log the error
            logFile = open( os.path.join( logDir, 'Plugins.' + platform + '.err' ), 'a' )
            logFile.write( 'PLUGIN CACHE : \nERROR GENERATING\n' )
            logFile.close()
        os.chdir( currentDirectory )
    return


def compilePythonModules( release_directory, platform ):
    if os.path.exists( os.path.join( 'config', 'cmt' ) ):
        print 'Compiling the python modules'
        currentDirectory = os.getcwd()
        os.chdir( os.path.join( 'config', 'cmt' ) )
        for pythonModule in coral_config.pythonModules:
            cmd = ''
            if string.find( platform, 'win32_vc' ) >= 0 :
                cmd = cmtsetup + 'setup.bat & python -c "import ' + pythonModule + '"'
            else:
                cmd = cmtsetup + '. ./setup.sh; python -c "import ' + pythonModule + '"'
            ret = os.system( cmd )
            if ret != 0:
                # Log the error
                logFile = open( os.path.join( logDir, 'PyModules.' + platform + '.err' ), 'a' )
                logFile.write( 'PYTHON MODULE : ' + pythonModule + ' \nERROR IMPORTING\n' )
                logFile.close()
        os.chdir( currentDirectory )
    return


def lockForConfig( release_directory, platform ):
    # Wait for other config locks to disappear
    while ( True ):
        existingLocks = False
        files = os.listdir( release_directory )
        for file in files:
            if string.find( file, 'config.lock.' ) == 0:
                existingLocks = True
                break
        if existingLocks:
            time.sleep(1)
        else:
            break

    # Create the config lock
    lockfile = open( os.path.join( release_directory, 'config.lock.' + platform ), 'w' )
    lockfile.close()

    # Wait for the make locks to disappear
    while ( True ):
        existingLocks = False
        files = os.listdir( release_directory )
        for file in files:
            if string.find( file, 'make.lock.' ) == 0:
                existingLocks = True
                break
        if existingLocks:
            time.sleep(1)
        else:
            break
    return


def unlockForConfig( release_directory, platform ):
    if os.path.exists( os.path.join( release_directory, 'config.lock.' + platform ) ) :
        os.remove( os.path.join( release_directory, 'config.lock.' + platform ) )
    return


def lockForMake( release_directory, platform, package ):
    while (True):
        # Wait for other config locks to disappear
        while ( True ):
            existingLocks = False
            files = os.listdir( release_directory )
            for file in files:
                if string.find( file, 'config.lock.' ) == 0:
                    existingLocks = True
                    break
            if existingLocks:
                time.sleep(1)
            else:
                break

        locked = False
        
        existingLocks = False
        files = os.listdir( release_directory )
        for file in files:
            if string.find( file, 'make.lock.' + package ) == 0:
                existingLocks = True
                break
        if existingLocks:
            time.sleep(3)
            continue

        # Create the make lock lock
        lockfile = open( os.path.join( release_directory, 'make.lock.' + package + '.' + platform ), 'w' )
        lockfile.close()

        # Wait for the make locks on the particular package to disappear
        for i in range( len(platform) + len(package) ):
            existingLocks = False
            files = os.listdir( release_directory )
            for file in files:
                if string.find( file, 'make.lock.' + package ) == 0 and file != 'make.lock.' + package + '.' + platform :
                    existingLocks = True
                    break
            if existingLocks:
                time.sleep(1)
            else:
                locked = True
                break
        if ( locked ) : break
        if os.path.exists( os.path.join( release_directory, 'make.lock.' + package + '.' + platform ) ) :
            os.remove( os.path.join( release_directory, 'make.lock.' + package + '.' + platform ) )
        time.sleep(2)
    return


def unlockForMake( release_directory, platform, package ):
    if os.path.exists( os.path.join( release_directory, 'make.lock.' + package + '.' + platform ) ) :
        os.remove( os.path.join( release_directory, 'make.lock.' + package + '.' + platform ) )
    return


def buildRelease( release_directory, platform ):
    lockForConfig( release_directory, platform )
    currentDirectory = os.getcwd()
    os.chdir( os.path.join( release_directory, 'src', 'config', 'cmt' ) )
    print 'Preparing to build...'
    ret = os.system( cmtsetup + 'cmt br cmt config' )
    os.chdir( currentDirectory )
    unlockForConfig( release_directory, platform )
    if ret != 0:
        raise Exception( 'Failed to setup platform ' + platform )
    os.chdir( os.path.join( release_directory, 'src' ) )
    for package in coral_config.packages:
        lockForMake( release_directory, platform, package[0] )
        buildPackage( package[0], platform )
        unlockForMake( release_directory, platform, package[0] )
    for testLibrary in coral_config.testLibraries:
        lockForMake( release_directory, platform, 'TestLib-' + testLibrary[0] )
        buildTestLibrary( testLibrary[0], platform )
        unlockForMake( release_directory, platform, 'TestLib-' + testLibrary[0] )
    for test in coral_config.tests:
        lockForMake( release_directory, platform, 'Test-' + test[0] )
        buildTest( test[0], platform )
        unlockForMake( release_directory, platform, 'Test-' + test[0] )
    refreshPlugins( release_directory, platform )
    compilePythonModules( release_directory, platform )
    os.chdir( currentDirectory )
    os.chdir( os.path.join( release_directory, 'src' ) )
    print 'Removing the temporary files...'
    removeTemporaries( platform )
    os.chdir( currentDirectory )
    return 0

# MAIN SCRIPT STARTS HERE

try:
    if len( sys.argv ) < 3:
        print 'Usage: ' + os.path.split( sys.argv[0] )[1] + ' release_directory platform'
    else:
        release_directory = sys.argv[1]
        if not ( os.path.exists( release_directory ) and os.path.isdir( release_directory ) ):
            print projectTop + ' does not exist or is not a directory'
        else:
            platform = sys.argv[2]
            logDir = os.path.join( release_directory, 'logs' )
            cmtsetup = setupEnvironment( release_directory, platform )
            ret = buildRelease( release_directory, platform )
            if ret != 0:
                raise Exception( 'Failed to build the release' )
            print 'Release built for platform ' + platform
        
except Exception, e:
    print "Exception : " + str( e )
    # release open config locks...
    unlockForConfig( release_directory, platform )
    # release open make locks
    files = os.listdir( release_directory )
    for file in files:
        if string.find( file, 'make.lock.' ) == 0 and string.find( file, '.' + platform ) > 1:
            os.remove( os.path.join( release_directory, file ) )
