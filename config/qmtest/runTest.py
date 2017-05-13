import sys
import os
import os.path
import shutil

def dirName( testName ):
  theDirName = testName
  if 'CMTCONFIG' in os.environ:
    theDirName = os.environ['CMTCONFIG'] + '.' + theDirName

  while ( True ):
    if os.path.exists( theDirName ):
      theDirName = '_' + theDirName
    else:
      break
  return theDirName



# Main script starts here

testName = sys.argv[1]
directoryName = dirName( testName )
os.mkdir( directoryName )

numberOfFilesToCopy = len(sys.argv) - 2
for i in range( numberOfFilesToCopy ):
  targetFile = sys.argv[ 2 + i ]
  if os.path.isfile( targetFile ):
    shutil.copy( targetFile, directoryName )
  else:
    shutil.copytree( targetFile, directoryName )

os.chdir( directoryName )
ret = os.system( testName )
os.chdir( os.pardir )
try:
  shutil.rmtree( directoryName )
except:
  pass # Do not fail test if directory cannot be removed (fix bug #64429)
if ret != 0:
  raise Exception, 'Test failed (return code='+repr(ret)+')'

