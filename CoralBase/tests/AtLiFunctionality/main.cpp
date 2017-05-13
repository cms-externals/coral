#include <iostream>
#include <exception>
#include "TestDriver.h"


int main( int, char** )
{
  try {
    TestDriver theTest;

    theTest.simpleAtLi();

    theTest.atLiWithSpec();

    theTest.atLiWithSharedSpec();

    theTest.atLiBound();

    theTest.atLiShared();
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
