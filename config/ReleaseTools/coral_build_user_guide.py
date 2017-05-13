#!/usr/bin/env python

import sys
import os
import os.path
import shutil

def generateComponentDoc( projectTop, component ):
    print 'Generating documentation for ' + component
    
    if os.path.exists( os.path.join( projectTop, 'doc', component ) ):
        shutil.rmtree( os.path.join( projectTop, 'doc', component ) )
    os.mkdir( os.path.join( projectTop, 'doc', component ) )
    
    # Link the xml fragments and the directory with the pictures
    currentWorkingDirectory = os.getcwd()
    os.chdir( os.path.join( projectTop, 'doc', component ) )
    srcDir = os.path.join( os.pardir, os.pardir, 'src', component, 'doc' )
    if os.path.exists( os.path.join( srcDir, component + '_pics' ) ):
        os.symlink( os.path.join( srcDir, component + '_pics' ), component + '_pics' )
    os.symlink( os.path.join( srcDir, component + '.xml' ), component + '.xml' )
    os.symlink( os.path.join( srcDir, 'Semantics.xml' ), 'Semantics.xml' )
    os.symlink( os.path.join( srcDir, 'Implementation.xml' ), 'Implementation.xml' )
    os.symlink( os.path.join( srcDir, 'ReferenceManual.xml' ), 'ReferenceManual.xml' )

    # Generate the documentation
    ret = os.system( 'export PATH=/afs/cern.ch/sw/XML/XMLBIN/bin/i386-linux:$PATH; docbook2html ' + component + '.xml' )
    if ret == 0:
        os.symlink( component + '.html', 'index.html' )
    os.chdir( currentWorkingDirectory )
    return ret





def includeAPIComponents( apiPackages ):

    dtdFile = open( 'Components.dtd', 'a' )
    for package in apiPackages:
        dtdFile.write( '<!ENTITY ' + package + 'Semantics SYSTEM "' + package + '/Semantics.xml">' )
        dtdFile.write( '<!ENTITY ' + package + 'Reference SYSTEM "' + package + '/ReferenceManual.xml">' )
    dtdFile.close()

    xmlFile = open( 'UserGuideComponents.xml', 'a' )

    xmlFile.write( '<section id="Public_API">\n' )
    xmlFile.write( '<title>Public API Description</title>\n' )
    for package in apiPackages:
        xmlFile.write( '&' + package + 'Semantics;\n' )
    xmlFile.write( '</section>' )

    xmlFile.write( '<section id="Public_API_Reference">\n' )
    xmlFile.write( '<title>Public API Reference</title>\n' )
    for package in apiPackages:
        xmlFile.write( '&' + package + 'Reference;\n' )
    xmlFile.write( '</section>' )

    xmlFile.close()
    return




def includeImplementationComponents( packages ):

    dtdFile = open( 'Components.dtd', 'a' )
    xmlFile = open( 'UserGuideComponents.xml', 'a' )

    xmlFile.write( '<section id="Implementation_Packages">\n' )
    xmlFile.write( '<title>Implementation Components</title>\n' )
    for package in packages:
        dtdFile.write( '<!ENTITY ' + package + 'Semantics SYSTEM "' + package + '/Semantics.xml">' )
        dtdFile.write( '<!ENTITY ' + package + 'Reference SYSTEM "' + package + '/ReferenceManual.xml">' )
        xmlFile.write( '&' + package + 'Semantics;\n' )
        xmlFile.write( '&' + package + 'Reference;\n' )
    xmlFile.write( '</section>' )

    dtdFile.close()
    xmlFile.close()
    return






def generateUserGuide( projectTop ):

    # Downloading the doxygen files
    cvsroot = ':kserver:coral.cvs.cern.ch:/cvs/coral'
    cvspackage = 'coral/doc/UserGuide'

    # Set up the output directories
    if not os.path.exists( os.path.join( projectTop, 'doc' ) ):
        os.mkdir( os.path.join( projectTop, 'doc' ) )
    if os.path.exists( os.path.join( projectTop, 'doc', 'UserGuide' ) ):
        shutil.rmtree( os.path.join( projectTop, 'doc', 'UserGuide' ) )
    if os.path.exists( os.path.join( projectTop, 'doc', 'pics' ) ):
        shutil.rmtree( os.path.join( projectTop, 'doc', 'pics' ) )
    if os.path.exists( os.path.join( projectTop, 'doc', 'UserGuide.html' ) ):
        os.remove( os.path.join( projectTop, 'doc', 'UserGuide.html' ) )
    docDir = os.path.join( projectTop, 'doc' )

    # Retrieve the old list of files
    listOfFilesAndDirs = os.listdir( docDir )
    listOfFilesAndDirs.append( 'UserGuide' )
    listOfFilesAndDirs.append( 'pics' )
    listOfFilesAndDirs.append( 'UserGuide.html' )


    # Generate the components documentation
    picDirs = []
    apiPackages = ( 'CoralBase', 'RelationalAccess' )

    implementationPackages = ( 'PyCoral','ConnectionService','RelationalService', 'OracleAccess', 'SQLiteAccess', 'MySQLAccess', 'FrontierAccess','XMLAuthenticationService', 'EnvironmentAuthenticationService', 'XMLLookupService', 'LFCLookupService', 'LFCReplicaService', 'MonitoringService', 'CoralCommon' )

    for package in apiPackages + implementationPackages:
        ret = generateComponentDoc( projectTop, package )
        if ret != 0:
            return ret
        if os.path.exists( os.path.join( docDir, package, package + '_pics' ) ):
            picDirs.append( package + '_pics' )
            currentWorkingDirectory = os.getcwd()
            os.chdir(docDir )
            if os.path.exists( package + '_pics' ):
                os.remove( package + '_pics' )
            os.symlink( os.path.join( package, package + '_pics' ), package + '_pics' )
            os.chdir( currentWorkingDirectory )
            listOfFilesAndDirs.append( package + '_pics' )
        listOfFilesAndDirs.append( package )


    # The User Guide
    print 'Building the User Guide'

    # Export the source
    currentWorkingDirectory = os.getcwd()
    os.chdir( projectTop )
    
    ret = os.system( 'cvs -d ' + cvsroot + ' export -r HEAD -d doc ' + cvspackage )
    if ret != 0:
        os.chdir( currentWorkingDirectory )
        return 1

    os.chdir( 'doc' )

    # Fill-in the components documentation
    includeAPIComponents( apiPackages )
    includeImplementationComponents( implementationPackages )

    # Run docbook
    ret = os.system( 'export PATH=/afs/cern.ch/sw/XML/XMLBIN/bin/i386-linux:$PATH; docbook2html -chunk UserGuide.xml' )
    if ret != 0:
        os.chdir( currentWorkingDirectory )
        return 1
    ret = os.system( 'export PATH=/afs/cern.ch/sw/XML/XMLBIN/bin/i386-linux:$PATH; docbook2html UserGuide.xml' )
    if ret != 0:
        os.chdir( currentWorkingDirectory )
        return 1

    # Create the symbolic link to the pics
    os.chdir( 'UserGuide' )
    os.symlink( os.path.join( os.pardir, 'pics' ), 'pics' )
    for picDir in picDirs:
        os.symlink( os.path.join( os.pardir, picDir ), picDir )

    # Remove the exported and temporarily created files
    os.chdir( docDir )
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
            ret = generateUserGuide( projectTop )
            if ret != 0:
                raise Exception( 'Failed to build the documentation' )
        
except Exception, e:
    print "Exception : " + str( e )
