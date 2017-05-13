#!/usr/bin/env python

import os
import os.path
import sys
import commands
import string
import shutil
import coral_config

cvsroot = ':kserver:coral.cvs.cern.ch:/cvs/coral'

cmtVersion = 'v1r18p20061003'
cmtReleaseDirectory='/afs/cern.ch/sw/contrib/CMT'
wincmtReleaseDirectory='z:\\cern.ch\\sw\\contrib\\CMT'

lcgcmtConfigParentDirectory = '/afs/cern.ch/sw/lcg/app/releases/LCGCMT'
winlcgcmtConfigParentDirectory = 'z:\\cern.ch\\sw\\lcg\\app\\releases\\LCGCMT'

logDir = ''

def setupEnvironment( release_directory, platform, workingDirectory ):
    cmtpathforlcg = ''
    tcmtsetup = ''
    if string.find( platform, 'win32_vc' ) >= 0:
        tcmtsetup = 'set SITEROOT=z:\\cern.ch& '
        cmtpathforlcg = os.path.join( release_directory, 'src' ) + os.pathsep + os.path.join( winlcgcmtConfigParentDirectory, coral_config.cmtLCGconfig )
        cmtpathforlcg = os.path.join( workingDirectory, '__testing__' + coral_config.releaseTag + '__' + platform, 'src' ) + os.pathsep + cmtpathforlcg
        tcmtsetup = tcmtsetup + os.path.join( wincmtReleaseDirectory, cmtVersion, 'mgr', 'setup.bat' ) + '& set CMTCONFIG=' + platform + '& '
        tcmtsetup = tcmtsetup + 'set CMTPATH=' + cmtpathforlcg + '& '
    else:
        tcmtsetup = 'export SITEROOT=/afs/cern.ch; '
        cmtpathforlcg = os.path.join( release_directory, 'src' ) + os.pathsep + os.path.join( lcgcmtConfigParentDirectory, coral_config.cmtLCGconfig )
        cmtpathforlcg = os.path.join( workingDirectory, '__testing__' + coral_config.releaseTag + '__' + platform, 'src' ) + os.pathsep + cmtpathforlcg        
        tcmtsetup = tcmtsetup + 'source ' + os.path.join( cmtReleaseDirectory, cmtVersion, 'mgr', 'setup.sh' ) + '; export CMTCONFIG=' + platform + '; '
        tcmtsetup = tcmtsetup + 'export CMTPATH=' + cmtpathforlcg + '; '
    return tcmtsetup


def createQmtestFile( testName ):
    qmtestFile = open( '___run_qmtest.qmt', 'w' )
    qmtestFile.write( '<?xml version="1.0" ?>\n' )
    qmtestFile.write( '<!DOCTYPE extension  PUBLIC "-//QM/2.3/Extension//EN"  "http://www.codesourcery.com/qm/dtds/2.3/-//qm/2.3/extension//en.dtd">\n' )
    qmtestFile.write( '<extension class="LCG_QMTestExtensions.ShellCommandTest" kind="test">\n' )
    qmtestFile.write( ' <argument name="excluded_lines">\n' )
    qmtestFile.write( '  <text />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="target_group">\n' )
    qmtestFile.write( '  <text>.*</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stderr">\n' )
    qmtestFile.write( '  <text>*</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdout">\n' )
    qmtestFile.write( '  <text />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="prerequisites">\n' )
    qmtestFile.write( '  <set />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdout_tol">\n' )
    qmtestFile.write( '  <integer>0</integer>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="exit_code">\n' )
    qmtestFile.write( '  <integer>0</integer>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdout_tag">\n' )
    qmtestFile.write( '  <text>[OVAL]</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="environment">\n' )
    qmtestFile.write( '  <set />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdout_ref_path">\n' )
    qmtestFile.write( '  <text>./</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="timeout">\n' )
    qmtestFile.write( '  <integer>-1</integer>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="description_test">\n' )
    qmtestFile.write( '  <text />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="resources">\n' )
    qmtestFile.write( '  <set />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdin">\n' )
    qmtestFile.write( '  <text />\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="command">\n' )
    qmtestFile.write( '  <text>test_' + testName + '</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( ' <argument name="stdout_ref">\n' )
    qmtestFile.write( '  <text>test_' + testName + '.ref</text>\n' )
    qmtestFile.write( ' </argument>\n' )
    qmtestFile.write( '</extension>\n' )
    qmtestFile.close()
    return '___run_qmtest'

def qmtestFile( testName ):
    fileName = ''
    files = os.listdir( os.curdir )
    for file in files:
        if os.path.isdir( file ) or os.path.islink( file ): continue
        i = string.find( file, '.qmt' )
        if i > 0:
            if len(file) - len('.qmt') - i == 0:
                fileName = file[:i]
                break
    if len(fileName) == 0:
        fileName = createQmtestFile( testName )
    return fileName


def runTests( release_directory, platform, workingDirectory ):
    cmtsetup = setupEnvironment( release_directory, platform, workingDirectory )
    currentDirectory = os.getcwd()
    os.chdir( workingDirectory )
    subDirectory = '__testing__' + coral_config.releaseTag + '__' + platform
    os.mkdir( subDirectory )
    os.chdir( os.path.join( workingDirectory, subDirectory ) )
    os.mkdir( 'src' )
    os.chdir( 'src' )

    # Check out the config/cmt and config/qmtest subdirectories
    cmd = cmd = 'cvs -d ' + cvsroot + ' export -r HEAD config/cmt'
    ret1 = os.system( cmd )
    cmd = cmd = 'cvs -d ' + cvsroot + ' export -r HEAD config/qmtest'
    ret2 = os.system( cmd )
    if ret1 != 0 or ret2 !=0 :
        os.chdir( workingDirectory )
        shutil.rmtree( subDirectory )
        os.chdir( currentDirectory )
        return 1;
    
    # Go to the cmt directory and issue cmt config
    os.chdir( os.path.join( 'config', 'cmt' ) )
    cmd = cmtsetup + 'cmt config'
    ret = os.system( cmd )
    if ret != 0:
        os.chdir( workingDirectory )
        shutil.rmtree( subDirectory )
        os.chdir( currentDirectory )
        return 1;

    # Update the command list
    if string.find( platform, 'win32_vc' ) >= 0:
        cmtsetup = cmtsetup + os.path.join( workingDirectory, subDirectory, 'src', 'config', 'cmt', 'setup.bat' ) + '& '
    else:
        cmtsetup = cmtsetup + 'source ' + os.path.join( workingDirectory, subDirectory, 'src', 'config', 'cmt', 'setup.sh' ) + '; '

    # Go to the working directory...
    os.chdir( os.path.join( workingDirectory, subDirectory, 'src' ) )

    # Loop over the test cases
    for testCase in coral_config.tests:
        testName = testCase[0]
        
        # Copy the source directory from the release
        print 'Setting up test ' + testName
        shutil.copytree( os.path.join( release_directory, 'src', 'Tests', testName ), testName )

        # Enter it and create a link to QMtest
        os.chdir( testName )
        os.symlink( os.path.join( workingDirectory, subDirectory, 'src', 'config', 'qmtest', 'QMTest' ), 'QMTest' )

        # Run qmtest against the qmt file
        print '    ...running ' + testName + '...'
        cmd = cmtsetup + 'qmtest run ' + qmtestFile( testName )
        ret = os.system( cmd )
        if ret != 0:
            print ' TEST FAILED!'
        else:
            print ' TEST OK'

        # Clean up the directory
        os.chdir( os.pardir )
        shutil.rmtree( testName )

    os.chdir( workingDirectory )
    shutil.rmtree( subDirectory )
    os.chdir( currentDirectory )
    return 0


# MAIN SCRIPT STARTS HERE

try:
    if len( sys.argv ) < 3:
        print 'Usage: ' + os.path.split( sys.argv[0] )[1] + ' release_directory platform [workingDirectory = pwd]'
    else:
        release_directory = sys.argv[1]
        if not ( os.path.exists( release_directory ) and os.path.isdir( release_directory ) ):
            print projectTop + ' does not exist or is not a directory'
        else:
            platform = sys.argv[2]
            workingDirectory = os.getcwd()
            if len(sys.argv) >= 4 and os.path.exists( sys.argv[3] ) and os.path.isdir( sys.argv[3] ):
                workingDirectory = sys.argv[3]
            logDir = os.path.join( workingDirectory, 'logs' )
            ret = runTests( release_directory, platform, workingDirectory )
            if ret != 0:
                raise Exception( 'Failed to run the tests' )
            print 'Tests run for platform ' + platform
        
except Exception, e:
    print "Exception : " + str( e )
