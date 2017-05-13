#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"

#include <iostream>
#include <cstdlib>

void testSharingLoop()
{
  coral::AttributeList al1;
  al1.extend("x", typeid(double));

  coral::AttributeList al2;
  al2.extend("y", typeid(double));

  coral::AttributeList al3;
  al3.extend("z", typeid(double));

  al1[0].shareData(al2[0]);
  al3[0].shareData(al2[0]);
  al1[0].shareData(al3[0]);
  // This should not crash. If the sharing does not prevent sharing loop,
  // the 3 lines below start an infinite loop of recursive function calls
  // causing segmentation fault.
  al1[0].addressOfData();
  al2[0].addressOfData();
  al3[0].addressOfData();

  // Check if sharing is real
  static const double VAL = 3.5;
  al1[0].data<double>() = VAL;

  if (al1[0].data<double>() != VAL || al2[0].data<double>() != VAL || al3[0].data<double>() != VAL) {
    std::cerr << "Data was not shared ..." << std::endl;
    exit(1);
  }
}

int main( int, char** )
{
  try {
    coral::AttributeList al;
    al.extend("x", typeid(double));
    coral::Attribute& attribute = al[0];
    attribute.data<double>() = 11.22;
    std::cout << attribute.specification().name() << " ("
              << attribute.specification().typeName() << ") : "
              << attribute.data<double>() << std::endl;
    // Test for throwing exception if the attribute is null
    attribute.setNull();
    bool thrown = false;

    try {
      // Must throw exception
      attribute.data<double>() = 11.22;
    } catch ( coral::AttributeException& ) {
      thrown = true;
    } catch (...) { throw; }

    if (!thrown) {
      std::cerr << "NULL attribute data access did not throw exception ..." << std::endl;
      return 1;
    }

    thrown = false;

    try {
      // also... (the const method)
      const coral::Attribute& cAttr = attribute;
      cAttr.data<double>();
    } catch ( coral::AttributeException& ) {
      thrown = true;
    } catch (...) { throw; }

    if (!thrown) {
      std::cerr << "NULL attribute data access did not throw exception ..." << std::endl;
      return 1;
    }

    testSharingLoop();
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
