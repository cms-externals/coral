#!/usr/bin/env python
import os, sys
from xml.dom import minidom, Node

def usage() :
    cmd = os.path.split(sys.argv[0])[1]
    print "Usage: " + cmd + " file1.xml [file2.xml...]"
    sys.exit(-1)

# See http://www.faqs.org/docs/diveintopython/kgp_parse.html
# See http://www.rexx.com/~dkuhlman/pyxmlfaq.html
# See http://pyxml.sourceforge.net/topics/howto/node20.html
def parseQmtestRun( file ) :
    ###print 'Parse qmtest run annotations from file:',file
    print "--- TEST ENVIRONMENT ---------------------------------------------------------\n"
    xmldoc = minidom.parse(file)
    for node in xmldoc.childNodes:
        if node.nodeType == Node.ELEMENT_NODE \
               and node.nodeName == "report" :
            report = node
            break
    for node in report.childNodes:
        if node.nodeType == Node.ELEMENT_NODE \
               and node.nodeName == "results" :
            results = node
            break
    # See http://docs.python.org/tut/node7.html
    # Ordered list with all required keys (keys)
    # Unordered dictionary with notes (notes)
    keys = []
    notes = {}
    keys.append("qmtest.run.uname")
    keys.append("qmtest.run.username")
    keys.append("qmtest.run.command_line")
    keys.append("qmtest.run.version")
    keys.append("qmtest.run.start_time")
    keys.append("qmtest.run.end_time")
    for node in results.childNodes:
        if node.nodeType == Node.ELEMENT_NODE \
               and node.nodeName == "annotation" :
            annotation = node
            ###print 'Node:', annotation
            ###print annotation.toxml()
            if annotation.attributes.get('key') is not None:
                key = annotation.attributes.get('key').value
                ###print '    %s:' % key
                if annotation.firstChild.nodeType == Node.TEXT_NODE:
                    data = annotation.firstChild.data
                    data = data.strip('\n')
                    ###print '      %s' % data            
                    notes[key] = data
    for key in keys :
        if notes.has_key(key) :
            print '  %s:' % key
            print '    %s\n' % notes[key]

##############################################################################

if __name__ == '__main__':
    if len(sys.argv) < 2 : usage()
    for file in sys.argv[1:] :
        parseQmtestRun( file )
