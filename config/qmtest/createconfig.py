#!/usr/bin/env python
#   @author Alexander Kalkhof
#   @date   2008-04-16

import sys, os
import xml.sax

#-----------------------------------------------------------------------------

def createQmtestTopDir(path):
    # Create directory <path> unless it already exists
    try: os.mkdir(path)
    except OSError: pass
    # Create directory <path>/QMtest unless it already exists
    try: os.mkdir(path + os.sep + 'QMTest')
    except OSError: pass
    # Create xml file <path>/QMtest/configuration
    fd = open(path + os.sep + 'QMTest' + os.sep + 'configuration', 'w')
    fd.write("<?xml version=\"1.0\" ?><!DOCTYPE extension  PUBLIC \'-//QM/2.3/Extension//EN\'  'http://www.codesourcery.com/qm/dtds/2.3/-//qm/2.3/extension//en.dtd'><extension class=\"xml_database.XMLDatabase\" kind=\"database\"/>")
    fd.close()

#-----------------------------------------------------------------------------

class Logger:	

    def __init__(self):
        self.fd = open("createconfig.log", 'w')

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def __del__(self):
        self.fd.close()	

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def msg(self, message):
        self.fd.write(message)
        self.fd.write("\n")

#-----------------------------------------------------------------------------

class BasicTest:
	
    def __init__(self, name, path, logger):
        self.logger = logger
        self.timeout = 300 # Timeout is now by default 5 min (fix bug #72986)
        self.backendPairs = []
        self.excludes = []
        self.name = name
        self.path = path
        self.pycoral = False
	
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def setTimeout(self, timeout):
        self.timeout = timeout
		
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def addBackendPair(self, writer, reader):
        self.backendPairs.append((writer, reader))
		
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def addExclude(self, platform, backendPair):
        self.excludes.append((platform, backendPair))
		
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def writeQmt(self, backendPair):
        self.logger.msg("\nTEST: %s" % self.name)		
        if backendPair:
            self.logger.msg("BACKENDPAIR (w,r): %s" % str(backendPair) )
        testname = self.name.split('_')
        # Build the backend-specific suffix and command line arguments
        if backendPair:
            if backendPair[1]:
                backend_args = backendPair[0] + ':' + backendPair[1]
                backend_suffix = '_' + backendPair[0] + '-' + backendPair[1]
            else:
                backend_args = backendPair[0]
                backend_suffix = '_' + backendPair[0]
        else:
            backend_args = ''
            backend_suffix = ''
        # Build the path to the reference output
        refpath = testname[1]
        for h in range(2, len(testname)): refpath += '_' + testname[h]
        refpath = ('../../Tests/%s/' % refpath) + '/'
        # Check if we have a reference output
        ref = False
        reffile = self.name + backend_suffix + '.ref'
        if os.path.exists(refpath + reffile):
            ref = True
        else:
            reffile = self.name + '.ref'
            if os.path.exists(refpath + reffile): ref = True
        # Start building the XML file contents
        xmloutput = ''
        xmloutput += '<?xml version="1.0" ?>'
        xmloutput += '<!DOCTYPE extension  PUBLIC "-//QM/2.3/Extension//EN"  "http://www.codesourcery.com/qm/dtds/2.3/-//qm/2.3/extension//en.dtd">\n'
        xmloutput += '<extension class="LCG_QMTestExtensions.ShellCommandTest" kind="test">\n'
        xmloutput += '<argument name="excluded_lines">'
        xmloutput += '<text />'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="target_group">'
        xmloutput += '<text>.*</text>'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="stderr">'
        xmloutput += '<text></text>' # OK only if stderr is empty (bug #73604)
        xmloutput += '</argument>\n'
        if not self.pycoral or self.name == "test_PyCoral_ImportPyCoral":
            xmloutput += '<argument name="prerequisites">'
            xmloutput += '<set />'
            xmloutput += '</argument>\n'
        else:
            xmloutput += '<argument name="prerequisites"><set>'
            xmloutput += '<tuple><text>pycoral.importpycoral</text><enumeral>PASS</enumeral></tuple>'
            xmloutput += '</set></argument>\n'
        if ref:
            xmloutput += '<argument name="stdout">'
            xmloutput += '<text />'
            xmloutput += '</argument>\n'
            xmloutput += '<argument name="stdout_ref">'
            xmloutput += '<text>%s</text>' % reffile
            xmloutput += '</argument>\n'
            xmloutput += '<argument name="stdout_ref_path">'
            xmloutput += '<text>%s</text>' % refpath
            xmloutput += '</argument>\n'
        else:
            xmloutput += '<argument name="stdout">'
            xmloutput += '<text>*</text>'
            xmloutput += '</argument>\n'
        xmloutput += '<argument name="stdout_tol">'
        xmloutput += '<integer>0</integer>'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="exit_code">'
        xmloutput += '<integer>0</integer>'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="stdout_tag">'
        xmloutput += '<text>[OVAL]</text>'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="environment"><set>\n'
        xmloutput += '<text>CORAL_MSGFORMAT=ATLAS</text>\n'
        xmloutput += '</set></argument>\n'
        if self.timeout:
            xmloutput += '<argument name="timeout">'
            xmloutput += '<integer>%s</integer>' % self.timeout
            xmloutput += '</argument>\n'
        xmloutput += '<argument name="description_test">'
        xmloutput += '<text />'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="command">'
        if not self.pycoral:
            xmloutput += '<text>%s %s</text>' % (self.name, backend_args)
        else:
            xmloutput += '<text>%s.py %s</text>' % (self.name, backend_args)
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="resources">'
        xmloutput += '<set />'
        xmloutput += '</argument>\n'
        xmloutput += '<argument name="stdin">'
        xmloutput += '<text />'
        xmloutput += '</argument>\n'
        # Process the list of excludes
        for ex in self.excludes:
            exclude = False
            exclude_platform = ex[0]
            exclude_backendPair = ex[1]
            # Exclude a platform (on all backends or on a specific backend)
            if exclude_platform:
                # Exclude it on all backends (including those in this test)
                if exclude_backendPair is None: # None means ANY here
                    exclude = True
                # Is the backend pair used in this test excluded?
                elif backendPair and exclude_backendPair == backendPair:
                    exclude = True
                # Is the RW backend used in this test excluded?
                elif backendPair and \
                         exclude_backendPair[0] == backendPair[0] and \
                         exclude_backendPair[1] is None: # None means ANY here
                    exclude = True
                # Is the RO backend used in this test excluded?
                elif backendPair and \
                         exclude_backendPair[1] == backendPair[1] and \
                         exclude_backendPair[0] is None: # None means ANY here
                    exclude = True
            # Exclude a backend on all platforms
            else:
                # Is the backend pair used in this test excluded?
                if backendPair and exclude_backendPair == backendPair:
                    exclude = True
                # Is the RW backend used in this test excluded?
                elif backendPair and \
                         exclude_backendPair[0] == backendPair[0] and \
                         exclude_backendPair[1] is None: # None means ANY here
                    exclude = True
                # Is the RO backend used in this test excluded?
                elif backendPair and \
                         exclude_backendPair[1] == backendPair[1] and \
                         exclude_backendPair[0] is None: # None means ANY here
                    exclude = True
            # We must exclude one or more platforms for this test
            # Regular expression '.*' excludes all platforms (any)x(>=0 times)
            if exclude:
                if exclude_platform is None: # None means ANY here
                    exclude_platform = '.*'
                self.logger.msg('EXCLUDE PLATFORM: %s' % exclude_platform)
                xmloutput += '<argument name="unsupported_platforms">'
                xmloutput += '<text>%s</text>' % exclude_platform
                xmloutput += '</argument>\n'
        # Finished building the XML file contents
        xmloutput += '</extension>\n'
        # Build the QMT file name
        qmtfile = testname[2]
        for h in range(3, len(testname)):
            qmtfile += '_' + testname[h]
        qmtfile += backend_suffix + '.qmt'
        qmtfile = self.path + os.sep + qmtfile.lower()
        # Create directory <testtype>.qms if it does not exist yet
        try: os.mkdir( self.path )
        except OSError: pass
        # Write the XML output to the QMT file
        ###print "Write file",qmtfile
        fd = open(qmtfile, 'w')
        fd.write(xmloutput)
        fd.close()

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def echo(self):
        print self.name
        print self.backendPairs
        print self.excludes

#-----------------------------------------------------------------------------

class IntegrationTest(BasicTest):
	
    def __init__(self, name, path, logger):
        BasicTest.__init__(self, name, path, logger)
				
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def write(self):
        if self.backendPairs:
            for i in self.backendPairs: self.writeQmt(i)

#-----------------------------------------------------------------------------

class PyCoralTest(BasicTest):
	
    def __init__(self, name, path, logger):
        if not name.endswith(".py") :
            print "ERROR! PyCoral test name does not end with '.py':", name
            exit(1)
        name=name[:-3] # strip trailing ".py"
        BasicTest.__init__(self, name, path, logger)
        self.pycoral = True
				
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def write(self):
        if self.backendPairs:
            for i in self.backendPairs: self.writeQmt(i)
        else:
            self.writeQmt(None)

#-----------------------------------------------------------------------------

class UnitTest(BasicTest):
	
    def __init__(self, name, path, logger):
        BasicTest.__init__(self, name, path, logger)

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def write(self):
        self.writeQmt(None)

#-----------------------------------------------------------------------------

# This class reads the testlist file and adds tests to the input TestSuite
class TestlistContentHandler(xml.sax.ContentHandler):
	
    def __init__(self, testsuite):
        self.testsuite = testsuite
        self.current_test_type = None
        self.current_test = None
	
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def startElement(self, name, attrs):       	
        #------------------
        # Parse 'testlist'
        #------------------
        if name == 'testlist': pass # dummy (single top-level container)
        #------------------
        # Parse 'testlist'
        #------------------
        elif name == 'testtype': 
            self.current_test_type = attrs.get('name',None)
            ###print "TestType:",self.current_test_type
            # Throw an exception if type is not specified
            if not self.current_test_type:
                print "ERROR! Test type is not specified for this test type"
                exit(1)
        #--------------
        # Parse 'test'
        #--------------
        elif name == 'test':
            test_name = attrs.get('name',None)
            test_timeout = attrs.get('timeout',None)
            # Throw an exception if type is not yet specified
            if not self.current_test_type:
                print "ERROR! Test type is not specified for this test"
                exit(1)
            # Throw an exception if type or name are not specified
            if not test_name or not self.current_test_type:
                print "ERROR! Test name is not specified for this test"
                exit(1)
            # Create and add to the suite the description of the current test
            self.current_test = self.testsuite.addTest( test_name, self.current_test_type )
            if test_timeout: self.current_test.setTimeout( test_timeout )
        #-----------------
        # Parse 'backend'
        #-----------------
        elif name == 'backend':
            backend_writer = attrs.get('writer',None)
            backend_reader = attrs.get('reader',None)
            ###print "-> Backend:",backend_writer,backend_reader
            if self.current_test: self.current_test.addBackendPair(backend_writer,backend_reader)
        #-----------------
        # Parse 'exclude'
        #-----------------
        elif name == 'exclude':
            exclude_platform = attrs.get('platform',None)
            exclude_backendPair = attrs.get('backend',None)
            ###print "-> Exclude:",exclude_platform,exclude_backendPair
            if self.current_test:
                if not exclude_backendPair :
                    self.current_test.addExclude(exclude_platform, None)
                else:
                    self.current_test.addExclude(exclude_platform, (exclude_backendPair,None))
                    self.current_test.addExclude(exclude_platform, (None,exclude_backendPair))
        #-----------------
        # Unknown element
        #-----------------
        else:
            print "ERROR! Unknown element in testfile:",name
            exit(1)

#-----------------------------------------------------------------------------

class TestSuite:
	
    def __init__(self, testlist, destpath):
        self.logger = Logger()
        self.tests = []
        self.path = destpath
        createQmtestTopDir( self.path )
        # Parse the "testlist.xml" file and fill the testsuite
        xmlparser = xml.sax.make_parser()
        xmlparser.setContentHandler( TestlistContentHandler(self) )
        xmlparser.parse( testlist )
        
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def addTest(self, test_name, test_type):
        # Create and add to the suite the TestDescription of the current test
        #test = TestDescription( self.path, test_name, test_type, self.logger )
        # Instantiate the appropriate concrete test for the given test type
        test_path = self.path + os.sep + test_type + ".qms"
        if test_type == 'integration':
            test = IntegrationTest( test_name, test_path, self.logger )
            test.addBackendPair( 'oracle', None )
            test.addBackendPair( 'mysql', None )
            test.addBackendPair( 'sqlite', None )
            test.addBackendPair( 'oracle', 'frontier' )
            test.addBackendPair( 'oracle', 'coral' )
            test.addBackendPair( 'mysql', 'coral' )
            test.addExclude( 'win', ('oracle', 'frontier') )
            test.addExclude( 'slc4,win', ('oracle', 'coral') )
            test.addExclude( None, ('mysql', 'coral') ) # bug #73834 and #81127
        elif test_type == 'pycoral':
            test = PyCoralTest( test_name, test_path, self.logger )
        elif test_type == 'pycoral_integration':
            test = PyCoralTest( test_name, test_path, self.logger )
            test.addBackendPair( 'oracle', None )
            test.addBackendPair( 'mysql', None )
            test.addBackendPair( 'sqlite', None )
            test.addBackendPair( 'oracle', 'frontier' )
            test.addBackendPair( 'oracle', 'coral' )
            test.addBackendPair( 'mysql', 'coral' )
            test.addExclude( 'win', ('oracle', 'frontier') )
            test.addExclude( 'slc4,win', ('oracle', 'coral') )
            test.addExclude( None, ('mysql', 'coral') ) # bug #73834
        elif test_type == 'unit':
            test = UnitTest( test_name, test_path, self.logger )
            if test_name.find('Frontier') >= 0: test.addExclude( 'win', None )
        elif test_type == 'unit_coralserver':
            test = UnitTest( test_name, test_path, self.logger )
            test.addExclude( 'slc4,win', None )
        else:
            # Throw an exception if type is unknown
            print "ERROR! Unknown test type", test_type
            exit(1)
        # Add the test to the test suite
        self.tests.append(test)		
        # Return a reference to the test (to add backends and excludes)
        return test
	
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def write(self):
        for i in self.tests: i.write()
            
    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def echo(self):
        for i in self.tests: i.echo()

#-----------------------------------------------------------------------------

def usage():
    sys.stderr.write( "Usage: " + sys.argv[0] +
                      "    : use existing config in '.' (do nothing)\n" )
    sys.stderr.write( "Usage: " + sys.argv[0] +
                      " -r : replace and use config in '.'\n" )    

# Analyse command line arguments (default for nightlies: no argument)
# No argument: print usage to stderr, do not change the config, return "."
# One argument "-r": replace the config in ".", return "."
# Other arguments: print usage to stderr and exit with an error
if len( sys.argv ) == 1:
    usage()
elif len( sys.argv ) == 2 and sys.argv[1] == "-r":
    # Parse testlist.xml and create all qms/qmt files in current directory
    pwd = os.path.realpath(os.curdir) + os.sep
    testsuite = TestSuite( 'testlist.xml', pwd )
    testsuite.write()
    ###testsuite.echo() # debug (dump the contents of the test suite)
else:
    sys.stderr.write( "ERROR! Invalid arguments to " + sys.argv[0] + "\n" )
    usage()
    exit(1)

# Return the path to the current directory ("qmtest -D `createconfig.py` run"
# is how the tests are launched in LCG_Builders/CORAL/scripts/CORAL_test.sh)
print '.' 
