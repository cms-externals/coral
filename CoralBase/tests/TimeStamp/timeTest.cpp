#include <iostream>
#include <exception>

#include "CoralBase/TimeStamp.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Exception.h"

/// This would fix bug #64016 on Windows (and also builds OK on Linux...
/// but why does it trigger no error from multiply defined symbols instead?)
//const boost::posix_time::ptime& coral::TimeStamp::Epoch = coral::getTimeStampEpoch();

int main( int, char** )
{
  try {

    std::cout << "Show Epoch" << std::endl;
    std::cout << "&(getTimeStampEpoch()) is " << &(coral::getTimeStampEpoch()) << std::endl;
    std::cout << "getTimeStampEpoch() is " << coral::getTimeStampEpoch() << std::endl;
    std::cout << "In timeTest.cpp: &(Epoch) is " << &(coral::TimeStamp::Epoch) << std::endl;
#ifndef _WIN32
    std::cout << "Epoch is " << coral::TimeStamp::Epoch << std::endl;
#endif

    // Test invariant with epoch time
    coral::TimeStamp epoch( coral::getTimeStampEpoch() );
    std::cout << "Nanoseconds from epoch should be 0 ns and is " << epoch.total_nanoseconds() << " ns " << std::endl;

    std::cout << "Boost fractional digits: " << boost::posix_time::time_duration::num_fractional_digits() << std::endl;
    coral::TimeStamp epoch123456789( 1970, 1, 1, 0, 0, 0, 123456789 );
    std::cout << "1. CORAL Nanoseconds from epoch should be 123456789 ns and is " << epoch123456789.total_nanoseconds() << " ns " << std::endl; // The result depends on the platform precision compiled in BOOST
    std::cout << "2. CORAL frac. seconds from epoch should be 123456789 ns and is " << epoch123456789.nanosecond() << " ns " << std::endl;

    coral::TimeStamp moment  = coral::TimeStamp::now();
    std::cout << "CORAL ns: " <<   moment.total_nanoseconds() << std::endl;

    coral::TimeStamp thisMoment = coral::TimeStamp::now(true);
    std::cout << "Testing the \""
              << coral::AttributeSpecification::typeNameForType( thisMoment )
              << "\" type..." << std::endl;

    std::cout << "The moment was "
              << thisMoment.day() << "/"
              << thisMoment.month() << "/"
              << thisMoment.year() << " "
              << thisMoment.hour() << ":"
              << thisMoment.minute() << ":"
              << thisMoment.second() << "."
              << thisMoment.nanosecond() << std::endl;

    coral::TimeStamp anotherMoment( 1999, 12, 21, 12, 10, 30, 213331 );
    std::cout << "Danae was born on " << anotherMoment.toString() << std::endl;
    std::cout << "Nanoseconds are " << anotherMoment.total_nanoseconds() << std::endl;

  }
  catch ( coral::Exception& e )
  {
    std::cerr << "CORAL Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e )
  {
    std::cerr << "C++ Exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... )
  {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] : Test successful" << std::endl;
  return 0;
}
