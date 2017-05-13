#!/usr/bin/env python

import sys
import os
import os.path

def releaseDirectory( directory ):
  currentWorkingDirectory = os.getcwd()
  os.chdir( directory )
  print 'Processing ' + directory
  ret = os.system( 'fs sa . system:anyuser rl' )
  os.chdir( currentWorkingDirectory )
  dirs = os.listdir( directory )
  for dir in dirs:
    subDirectory = os.path.join( directory, dir )
    if os.path.isdir( subDirectory ):
      releaseDirectory( subDirectory )
  return


try:
  if len( sys.argv ) < 2:
    raise Exception( 'Directory not specified' )
  releaseDirectory( sys.argv[1] )

except Exception, e:
    print "Exception : " + str( e )
