// Include files
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralBase/Exception.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/RelationalServiceException.h"

namespace coral
{

  class ConnectionTest : public CoralCppUnitTest
  {

    CPPUNIT_TEST_SUITE( ConnectionTest );
    CPPUNIT_TEST( test );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void setUp()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void test()
    {
      coral::ConnectionService connSvc;
      // Set configuration parameters
      coral::IConnectionServiceConfiguration& config = connSvc.configuration();
      config.setConnectionRetrialPeriod(1);
      config.setConnectionRetrialTimeOut(10);

      coral::ISessionProxy* proxy = connSvc.connect( "CORAL-Oracle-lcgnight/reader", coral::ReadOnly );

      proxy->transaction().start( true ); // readonly

      std::cout << "List tables in the schema" << std::endl;
      std::set< std::string > tables = proxy->nominalSchema().listTables();
      std::cout << "Retrieved " << tables.size() << " tables:" << std::endl;
      unsigned int i = 0;
      for ( std::set< std::string >::const_iterator iTable = tables.begin(); iTable != tables.end(); ++iTable, ++i ) 
      {
        if ( i<10 || i>tables.size()-10 ) 
          std::cout << "  " << *iTable << std::endl;
        else if ( i==10 )
          std::cout << "  ..." << std::endl;
      }
      
      /** get proxy providing logical service name
          connection with same credentials exists and access mode is ReadOnly,
          new physical connection will not be opened*/

      coral::ISessionProxy* proxy_read = connSvc.connect( "CORAL-Oracle-lcgnight/reader", coral::ReadOnly );

      proxy_read->transaction().start( true );

      proxy->transaction().commit();

      ///** physical connection is shared */
      //cout <<"connection shared: "<<proxy_read->isConnectionShared()<<endl;
      delete proxy;
      delete proxy_read;

      /** New connection will be opened since the access mode of original
          physical connection is ReadOnly */

      coral::ISessionProxy* proxy1 = connSvc.connect( "CORAL-Oracle-lcgnight/admin" );

      proxy1->transaction().start();

      tables = proxy1->nominalSchema().listTables();
      std::cout << "Retrieved " << tables.size() << " tables:" << std::endl;
      i = 0;
      for ( std::set< std::string >::const_iterator iTable = tables.begin(); iTable != tables.end(); ++iTable, ++i ) 
      {
        if ( i<10 || i>tables.size()-10 ) 
          std::cout << "  " << *iTable << std::endl;
        else if ( i==10 )
          std::cout << "  ..." << std::endl;
      }

      std::set<std::string> cppTypes = proxy1->typeConverter().supportedCppTypes();
      std::cout << "List known C++ types" << std::endl;
      for ( std::set<std::string>::const_iterator iType = cppTypes.begin();
            iType != cppTypes.end(); ++iType )
      {
        std::cout << *iType << " -> "
                  << proxy1->typeConverter().sqlTypeForCppType( *iType ) 
                  << std::endl;
      }

      proxy1->transaction().commit();
      /** New physical connection will be opened because both sessions are in 'Update'
          mode */
      coral::ISessionProxy* proxy_write = connSvc.connect( "CORAL-Oracle-lcgnight/admin" );

      delete proxy1;
      delete proxy_write;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void
    tearDown()
    {
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  };

  CPPUNIT_TEST_SUITE_REGISTRATION( ConnectionTest );

}

CORALCPPUNITTEST_MAIN( ConnectionTest )
