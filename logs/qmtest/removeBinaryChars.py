#!/usr/bin/env python
import os, sys, shutil

def usage() :
    cmd = os.path.split(sys.argv[0])[1]
    print "Usage: " + cmd + " oldfile newfile"
    sys.exit(-1)

def removeBinaryChars( file1, file2 ) :
    #print "Remove binary characters from", file1
    #print "Results will be in", file2
    f1 = open( file1 )
    f2 = open( file2, 'w' )
    for line1 in f1:
        line1 = line1[0:len(line1)-1] # Strip trailing "\n"
        line2 = ""
        for char1 in line1: line2 += printable(char1)
        f2.write( line2 + "\n" )
    f1.close()
    f2.close()
    
##############################################################################

# See http://www.python.org/dev/peps/pep-3138
# Expect to get a single character (but do not check...)
def printable( char ):
    if char == "'":
        # Do not process through repr (it would wrap double quotes around "'")
        return char
    elif char == '\r' or char == '\n' or char == '\t' or char == '\\' :
        # Do not process through repr (it would escape them)
        return char
    else :
        # Strip single quotes (repr only wraps double quotes around "'")
        return repr(char).strip("'")

if __name__ == '__main__':
    if len(sys.argv) != 3 : usage()
    file1 = sys.argv[1]
    file2 = sys.argv[2]
    removeBinaryChars( file1, file2 )
    ###for i in range(0,256) : print i, printable(chr(i)) # TEST!
