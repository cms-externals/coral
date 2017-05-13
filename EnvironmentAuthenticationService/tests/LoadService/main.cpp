#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"

#include "CoralBase/Exception.h"

#include "CoralKernel/Context.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

int main( int, char** )
{
  try {
    char c_user[] = "CORAL_AUTH_USER=dbuser";
    char c_pwd[]  ="CORAL_AUTH_PASSWORD=dbpasswd";
#ifndef _WIN32
    ::putenv( c_user );
    ::putenv( c_pwd );
#else
    ::_putenv( c_user );
    ::_putenv( c_pwd );
#endif

    std::vector< std::string > connections;
    connections.push_back( "oracle://host1/USER" );
    connections.push_back( "oracle://host2/USER" );

    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/EnvironmentAuthenticationService" );

    coral::IHandle<coral::IAuthenticationService> authSvc = context.query<coral::IAuthenticationService>();

    if ( authSvc.isValid() )
    {
      for ( std::vector< std::string >::const_iterator iConnection = connections.begin();
            iConnection != connections.end(); ++iConnection ) {
        std::cout << "Connection \"" << *iConnection << "\" :" << std::endl;
        const coral::IAuthenticationCredentials& credentials = authSvc->credentials( *iConnection );
        int numberOfItems = credentials.numberOfItems();
        for ( int i = 0; i < numberOfItems; ++i ) {
          std::string itemName = credentials.itemName( i );
          std::cout << "    " << itemName << " : " << credentials.valueForItem( itemName ) << std::endl;
        }
      }
    }
    else {
      throw std::runtime_error("Could not retrieve an authentication service.");
    }

    std::cout << "Exiting..." << std::endl;
  }
  catch ( coral::Exception& se ) {
    std::cerr << "CORAL Exception : " << se.what() << std::endl;
    return 1;
  }
  catch ( std::exception& e ) {
    std::cerr << "Standard C++ exception : " << e.what() << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "Exception caught (...)" << std::endl;
    return 1;
  }
  std::cout << "[OVAL] Success" << std::endl;
  return 0;
}
