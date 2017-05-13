#!/usr/bin/env python

import os
import os.path

import coral_config

# MAIN SCRIPT STARTS HERE

try:
    cvsroot = ':kserver:coral.cvs.cern.ch:/cvs/coral'
    tagCommand = 'cvs -d ' + cvsroot + ' rtag -r '
    for package in coral_config.packages:
        moduleName = package[0]
        moduleTag = package[0] + '-' + package[1]
        cmd = tagCommand + moduleTag + ' ' + coral_config.releaseTag + ' ' + moduleName
        ret = os.system( cmd )
    for package in coral_config.testLibraries:
        moduleName = os.path.join( 'Tests', 'Libraries', package[0] )
        moduleTag = 'Tests-Libraries-' + package[0] + '-' + package[1]
        cmd = tagCommand + moduleTag + ' ' + coral_config.releaseTag + ' ' + moduleName
        ret = os.system( cmd )        
    for package in coral_config.tests:
        moduleName = os.path.join( 'Tests', package[0] )
        moduleTag = 'Tests-' + package[0] + '-' + package[1]
        cmd = tagCommand + moduleTag + ' ' + coral_config.releaseTag + ' ' + moduleName
        ret = os.system( cmd )
    cmd = tagCommand + 'HEAD ' + coral_config.releaseTag + ' Tests/cmt'
    ret = os.system( cmd )
    cmd = tagCommand + 'HEAD ' + coral_config.releaseTag + ' doc'
    ret = os.system( cmd )
    cmd = tagCommand + 'HEAD ' + coral_config.releaseTag + ' coral/config'
    ret = os.system( cmd )
    cmd = tagCommand + 'HEAD ' + coral_config.releaseTag + ' coral/cmt'
    ret = os.system( cmd )
        
except Exception, e:
    print "Exception : " + str( e )
