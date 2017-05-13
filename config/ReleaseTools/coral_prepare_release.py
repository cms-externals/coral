#!/usr/bin/env python

import os
import os.path
import sys
import shutil

import coral_config


# Helper method to remove all the contents of a directory if it exists
def emptyDir( dirName ):
    if not os.path.exists( dirName ) : return
    currentDirectory = os.getcwd()
    os.chdir( dirName )
    directoryContents = os.listdir( os.getcwd() )
    for content in directoryContents:
        if os.path.isdir( content ):
            shutil.rmtree( content )
        else:
            os.remove( content )
    os.chdir( currentDirectory )
    return


# Method which bootstraps the release area
def bootStrapArea( releaseParentDirectory ):
    if not os.path.exists( os.path.join( releaseParentDirectory, coral_config.releaseTag ) ):
        os.makedirs( os.path.join( releaseParentDirectory, coral_config.releaseTag ) )
    os.makedirs( os.path.join( releaseParentDirectory, coral_config.releaseTag, 'src' ) )
    os.makedirs( os.path.join( releaseParentDirectory, coral_config.releaseTag, 'logs' ) )
    os.makedirs( os.path.join( releaseParentDirectory, coral_config.releaseTag, 'doc' ) )
    return 0


# Export the header files
def exportHeaderFiles( projectDir ):
    currentDir = os.getcwd()
    includeDir = os.path.join( projectDir, 'include' )
    os.mkdir( includeDir )
    os.chdir( includeDir )
    for package in coral_config.publicPackages:
        shutil.copytree( os.path.join( os.pardir, 'src', package, package ), package )
    os.chdir( currentDir )
    return 0


# Method which exports the code
def checkoutSources( sourceDirectory ):
    cvsroot = ':kserver:coral.cvs.cern.ch:/cvs/coral'
    # The packages
    for package in coral_config.packages:
        packageName = package[0]
        packageTag = package[0] + '-' + package[1]
        cmd = 'cvs -d ' + cvsroot + ' export -r ' + packageTag + ' ' + packageName
        currentDirectory = os.getcwd()
        os.chdir( sourceDirectory )
        ret = os.system( cmd )
        os.chdir( currentDirectory )
        if ret != 0: return ret
    # The Test Libraries
    for package in coral_config.testLibraries:
        packageName = os.path.join( 'Tests', 'Libraries', package[0] )
        packageTag = 'Tests-Libraries-' + package[0] + '-' + package[1]
        cmd = 'cvs -d ' + cvsroot + ' export -r ' + packageTag + ' ' + packageName
        currentDirectory = os.getcwd()
        os.chdir( sourceDirectory )
        ret = os.system( cmd )
        os.chdir( currentDirectory )
        if ret != 0: return ret
    # The Tests
    for package in coral_config.tests:
        packageName = os.path.join( 'Tests', package[0] )
        packageTag = 'Tests-' + package[0] + '-' + package[1]
        cmd = 'cvs -d ' + cvsroot + ' export -r ' + packageTag + ' ' + packageName
        currentDirectory = os.getcwd()
        os.chdir( sourceDirectory )
        ret = os.system( cmd )
        os.chdir( currentDirectory )
        if ret != 0: return ret
    # The Tests cmt
    cmd = 'cvs -d ' + cvsroot + ' export -r HEAD ' + os.path.join( 'Tests', 'cmt' )
    currentDirectory = os.getcwd()
    os.chdir( sourceDirectory )
    ret = os.system( cmd )
    os.chdir( currentDirectory )
    if ret != 0: return ret
    # The Project cmt
    cmd = 'cvs -d ' + cvsroot + ' export -r HEAD cmt'
    currentDirectory = os.getcwd()
    os.chdir( sourceDirectory )
    ret = os.system( cmd )
    os.chdir( currentDirectory )
    if ret != 0: return ret
    # The config directory
    cmd = 'cvs -d ' + cvsroot + ' export -r HEAD -d config ' + os.path.join( 'coral', 'config' )
    currentDirectory = os.getcwd()
    os.chdir( sourceDirectory )
    ret = os.system( cmd )
    os.chdir( currentDirectory )
    if ret != 0: return ret
    return exportHeaderFiles( os.path.join( sourceDirectory, os.pardir ) )

# Prepares the release area
def prepareReleaseArea( releaseParentDirectory ):
    releaseArea = os.path.join( releaseParentDirectory, coral_config.releaseTag )
    print 'Preparing release area : ' + releaseArea    
    emptyDir( releaseArea )
    ret = bootStrapArea( releaseParentDirectory )
    if ret != 0 : raise Exception( 'Failed to bootstrap the release area' )
    ret = checkoutSources( os.path.join( releaseArea, 'src' ) )
    if ret != 0 : raise Exception( 'Failed to export the sources' )
    print 'Prepared release area : ' + releaseArea    
    return ret


# MAIN SCRIPT STARTS HERE

try:
    if len( sys.argv ) < 2:
        print 'Usage: ' + os.path.split( sys.argv[0] )[1] + ' release_parent_directory'
    else:
        release_parent_directory = sys.argv[1]
        if not ( os.path.exists( release_parent_directory ) and os.path.isdir( release_parent_directory ) ):
            print projectTop + ' does not exist or is not a directory'
        else:
            ret = prepareReleaseArea( release_parent_directory )
            if ret != 0:
                raise Exception( 'Failed to prepare the release area' )
        
except Exception, e:
    print "Exception : " + str( e )
