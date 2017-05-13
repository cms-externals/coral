#include "../../src/NamedInputParametersParser.h"

#include <string>
#include <iostream>

int main( int, char** )
{
  try
  {
    std::string input1( "UPDATE COOL_SVEN.\"COOLTEST_NODES_SEQ\" SET \"CURRENT_VALUE\"=:\"newvalue\", \"LASTMOD_DATE\"=DATE_FORMAT(now(),'%Y-%m-% d_%H:%i:%s.000000000 GMT')" );
    std::string input2( "INSERT INTO T ( ID, sc ) VALUES ( :\"ID\", :\"sc\", :\"uc\" )" );

    std::string orig = input1;

    coral::MySQLAccess::NamedInputParametersParser nipp;

    nipp.bindVariable( "newvalue", "0", input1 );

    std::cout << "Original input1: " << orig << std::endl;
    std::cout << "Updated input:   " << input1 << std::endl;

    orig = input2;

    nipp.bindVariable( "ID", "0"     , input2 );
    nipp.bindVariable( "sc", "'\\\\'" , input2 );
    nipp.bindVariable( "uc", "255"   , input2 );

    std::cout << "Original input2: " << orig   << std::endl;
    std::cout << "Updated input:   " << input2 << std::endl;
  }
  catch( const std::exception& e )
  {
    std::cerr << "Standard excption has been caught: " << e.what() << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Ooops, the real problem here, unknown exception has been caught..." << std::endl;
    return 2;
  }

  return 0;
}
