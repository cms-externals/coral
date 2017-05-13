#include "RelationalAccess/IRelationalService.h"
#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"

#include <iostream>
#include <stdexcept>

int main( int, char** )
{
  try {
    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/RelationalService" );

    coral::IHandle<coral::IRelationalService> iSvc = context.query<coral::IRelationalService>();

    if ( ! iSvc.isValid() )
    {
      throw std::runtime_error( "Could not locate the relational service" );
    }

    std::vector< std::string > technologies = iSvc->availableTechnologies();
    std::cout << "Technologies : " << std::endl;
    for ( std::vector< std::string >::const_iterator iTechnology = technologies.begin();
          iTechnology != technologies.end(); ++iTechnology )
      std::cout << "     " << *iTechnology << std::endl;

  }
  catch ( coral::Exception& e ) {
    std::cerr << "CORAL exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "C++ exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Funny error ..." << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
