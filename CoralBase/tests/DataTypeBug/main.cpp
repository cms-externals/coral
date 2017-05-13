#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"

#include <iostream>
#include <exception>
#include <string>

int main ( int, char** )
{
  try
  {
    coral::AttributeList al;

    al.extend( "str", typeid(std::string) );

    al["str"].data<std::string>() = "valid";

    al.toOutputStream( std::cout ); std::cout << std::endl << std::endl;

    al["str"].data<int>() = 0;

    al.toOutputStream( std::cerr ); std::cerr << std::endl << std::endl;
  }
  catch ( const std::exception& e )
  {
    std::cerr << "Caught a standard exception: " << e.what() << std::endl;
    return 2;
  }
  catch ( ... )
  {
    std::cerr << "Caught a weird exception!" << std::endl;
    return 3;
  }

  return 0;
}
