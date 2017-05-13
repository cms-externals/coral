// $Id: CppUnit_headers.h,v 1.1 2010/05/18 14:47:08 avalassi Exp $
#ifndef COMMON_CPPUNIT_HEADERS_H
#define COMMON_CPPUNIT_HEADERS_H 1

// Disable warnings triggered by the CppUnit headers
// See http://wiki.services.openoffice.org/wiki/Writing_warning-free_code
// See also http://www.artima.com/cppsource/codestandards.html
// See also http://gcc.gnu.org/onlinedocs/gcc-4.1.1/cpp/System-Headers.html
// See also http://gcc.gnu.org/ml/gcc-help/2007-01/msg00172.html
#if defined __GNUC__
#pragma GCC system_header
#endif

// Needed for the testdriver (derived from the initial SPI version)
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>

// Needed for producing one line per test
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>

// Needed for the implementation of the COOL/CORAL ProgressListener
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestListener.h>

// Macros for the tests
#include <cppunit/extensions/HelperMacros.h>

#endif // COMMON_CPPUNIT_HEADERS_H
