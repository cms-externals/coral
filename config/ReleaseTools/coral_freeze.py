#!/usr/bin/env python

import sys
import os
import os.path
import commands
import string
import shutil

def parseArguments( args ):

    if len(args) < 2:
        print 'Usage: ' + os.path.split( args[0] )[1] + ' topDirectory'
        return None

    # Retrieve the release directory
    projectTop = args[1]
    
    return projectTop


def freezeArea( projectTop ):
    if not os.path.exists( projectTop ) : return
    currentWorkingDirectory = os.getcwd()
    os.chdir( projectTop )

    # Remove the tmp directory
    if os.path.exists( 'tmp' ):
        shutil.rmtree( 'tmp' )

    # Retrieve the current access rights
    cmdString = 'fs la'
    fslaOutput = commands.getstatusoutput( cmdString )
    users = ( string.split( fslaOutput[1], '\n' ) )[2:]
    for user in users :
        us = ( string.split( user ) )[0]
        cmd = 'fs sa . ' + us + ' rl'
        if us in ('lcgapp:coraladm', 'lcgapp:spiadm', 'system:administrators'):
            cmd = cmd + 'a'
        ret = commands.getstatusoutput( cmd )
        if ret[0] != 0 :
            os.chdir( currentWorkingDirectory )
            raise Exception( 'AFS-Freeze : ' + ret[1] )
    cmd = 'find . -type d -exec fs copyacl -clear -fromdir . -todir {} \;'
    ret = commands.getstatusoutput( cmd )
    os.chdir( currentWorkingDirectory )
    if ret[0] != 0 :
        raise Exception( 'AFS-Freeze : ' + ret[1] )
    return

# MAIN SCRIPT STARTS HERE

try:
    projectTop = parseArguments( sys.argv )
    if projectTop != None:
        freezeArea( projectTop )
        
except Exception, e:
    print "Exception : " + str( e )
