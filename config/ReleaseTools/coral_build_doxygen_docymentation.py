#!/usr/bin/env python

import sys
import os
import os.path
import shutil

def generateDoxygenOutput( projectTop ):

    # Downloading the doxygen files
    cvsroot = ':kserver:coral.cvs.cern.ch:/cvs/coral'
    cvspackage = 'coral/config/doxygen'

    # Set up the output directories
    if not os.path.exists( os.path.join( projectTop, 'doc' ) ):
        os.mkdir( os.path.join( projectTop, 'doc' ) )
    if os.path.exists( os.path.join( projectTop, 'doc', 'doxygen' ) ):
        shutil.rmtree( os.path.join( projectTop, 'doc', 'doxygen' ) )
    docDir = os.path.join( projectTop, 'doc' )

    # Retrieve the old list of files
    listOfFilesAndDirs = os.listdir( docDir )
    listOfFilesAndDirs.append( 'doxygen' )

    # Export the source
    currentWorkingDirectory = os.getcwd()
    os.chdir( projectTop )
    ret = os.system( 'cvs -d ' + cvsroot + ' export -r HEAD -d doc ' + cvspackage )
    if ret != 0:
        os.chdir( currentWorkingDirectory )
        return 1

    # Scan the sources to find the input
    libraries = []
    excludedDirs = ( 'CVS', 'Tests', 'Examples', 'doc', 'config', 'contrib' )
    if not os.path.exists( os.path.join( projectTop, 'src' ) ):
        return 1
    listOutput = os.listdir( os.path.join( projectTop, 'src' ) )
    for output in listOutput:
        if ( output not in excludedDirs ) and os.path.isdir( os.path.join( projectTop, 'src', output ) ):
            libraries.append( output )

    # Modify the Doxyfile
    logFile = open( os.path.join( docDir, 'Doxyfile' ), 'a' )
    logFile.write( '\n' )
    logFile.write( 'INPUT =' )
    for library in libraries:
        libTopDir = os.path.join( projectTop, 'src', library )
        libInclude = os.path.join( libTopDir, library )
        if os.path.exists( libInclude ):
            logFile.write( ' \\\n      ' + libInclude )
        libSrc = os.path.join( libTopDir, 'src' )
        if os.path.exists( libSrc ):
            logFile.write( ' \\\n      ' + libSrc )
    logFile.write( '\n' )
    logFile.close()
    
    # Produce the Doxygen output
    os.chdir( docDir )
    ret = os.system( '/afs/cern.ch/sw/lcg/external/doxygen/1.4.1/slc3_ia32_gcc323/bin/doxygen Doxyfile' )
    os.rename( 'html', 'doxygen' )
    shutil.copyfile( 'LCG_logo.jpg', os.path.join( 'doxygen', 'LCG_logo.jpg' ) )

    # Remove the exported and temporarily created files
    listOfFilesAndDirsNew = os.listdir( docDir )
    for fileOrDir in listOfFilesAndDirsNew:
        if fileOrDir not in ( listOfFilesAndDirs ):
            fullPath = os.path.join( docDir, fileOrDir )
            if os.path.isdir( fullPath ):
                shutil.rmtree( fullPath )
            else:
                os.remove( fullPath )

    os.chdir( currentWorkingDirectory )

    return ret
    ######################### THE END ##########################################



# MAIN SCRIPT STARTS HERE

try:
    if len( sys.argv ) < 2:
        print 'Usage: ' + os.path.split( sys.argv[0] )[1] + ' release_directory'
    else:
        projectTop = sys.argv[1]
        if not ( os.path.exists( projectTop ) and os.path.isdir( projectTop ) ):
            print projectTop + ' does not exist or is not a directory'
        else:
            ret = generateDoxygenOutput( projectTop )
            if ret != 0:
                raise Exception( 'Failed to generate doxygen documentation' )
        
except Exception, e:
    print "Exception : " + str( e )
