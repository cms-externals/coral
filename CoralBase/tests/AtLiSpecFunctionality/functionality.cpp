#include <iostream>
#include <exception>

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/AttributeListSpecification.h"


int main( int, char** )
{
  try {
    coral::AttributeListSpecification* spec = new coral::AttributeListSpecification;

    spec->extend<double>( "x" );
    spec->extend( "y", typeid(float) );
    spec->extend( "z", "int" );

    for ( coral::AttributeListSpecification::const_iterator iSpec = spec->begin();
          iSpec != spec->end(); ++iSpec ) {
      std::cout << iSpec->name() << " : " << iSpec->typeName() << std::endl;
    }

    const coral::AttributeSpecification& atSpec = spec->specificationForAttribute( spec->index( "y" ) );
    std::cout << "Type for \"y\" is \"" << atSpec.typeName() << "\"" << std::endl;

    spec->release();
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
