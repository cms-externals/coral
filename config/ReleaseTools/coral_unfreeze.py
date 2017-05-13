#!/usr/bin/env python

import sys
import os
import os.path
import commands

try:
    if len( sys.argv ) < 2 :
        print 'Syntax : ' + sys.argv[0] + ' directories'
    else :
        currentWorkingDirectory = os.getcwd()
        for idir in range( len( sys.argv ) - 1 ):
            directory = sys.argv[idir+1]
            if ( os.path.isdir( directory ) ):
                os.chdir( directory )
                cmdString = 'fs la'
                fslaOutput = commands.getstatusoutput( cmdString )
                if fslaOutput[0] == 0 :
                    print 'Un freezing the afs directory ' + directory
                    cmd = 'fs sa . lcgapp:coraladm rlidwka'
                    ret = commands.getstatusoutput( cmd )
                    cmd = 'find . -type d -exec fs copyacl -clear -fromdir . -todir {} \;'
                    ret = commands.getstatusoutput( cmd )
                else:
                    print 'No afs directory : ' + directory
            else:
                print 'No directory ' + directory

        os.chdir( currentWorkingDirectory )

except Exception, e:
    print 'Exception : ' + str( e )
