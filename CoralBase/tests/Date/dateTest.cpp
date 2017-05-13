#include <iostream>
#include <exception>
#include "CoralBase/Date.h"
#include "CoralBase/AttributeSpecification.h"

int main( int, char** )
{
  try {
    std::cout << "Testing the \""
              << coral::AttributeSpecification::typeNameForType<coral::Date>()
              << "\" type..." << std::endl;
    coral::Date theDay = coral::Date::today();
    std::cout << "The date today is "
              << theDay.day() << "/"
              << theDay.month() << "/"
              << theDay.year() << std::endl;

    coral::Date anotherDay( 1999, 12, 21 );
    std::cout << "Danae was born on "
              << anotherDay.day() << "/"
              << anotherDay.month() << "/"
              << anotherDay.year() << std::endl;
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Unknown exception ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] : Test successful" << std::endl;
  return 0;
}
