#include <iostream>
#include <string>
#include <sstream>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"

#include <cstdlib>
#include <memory>
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/Utilities.h"
#include "CoralKernel/Context.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/RelationalServiceException.h"

namespace
{

  //FIXME, the following methods should be moved to a common space

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  /* get a new session from service description and credentials */
  coral::ISession* getSession( const coral::IDatabaseServiceDescription& svdesc,
                               const coral::IAuthenticationCredentials& creds )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();
    // Load the oracle component
    context.loadComponent( "CORAL/RelationalPlugins/oracle" );
    coral::IHandle<coral::IRelationalDomain> iHandle =
      context.query<coral::IRelationalDomain>( "CORAL/RelationalPlugins/oracle" );
    if ( ! iHandle.isValid() )
    {
      throw coral::NonExistingDomainException( "oracle" );
    }

    std::pair<std::string, std::string> connectionAndSchema =
      iHandle->decodeUserConnectionString( svdesc.connectionString() );

    coral::IConnection* connection = iHandle->newConnection( connectionAndSchema.first );

    if ( ! connection->isConnected() )
      connection->connect();

    coral::ISession* session = connection->newSession( connectionAndSchema.second );
    if ( session )
    {
      session->startUserSession( creds.valueForItem( creds.userItem() ),
                                 creds.valueForItem( creds.passwordItem() ) );
    }
    return session;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  const coral::IDatabaseServiceDescription&
  getServiceDescription( const std::string& connectionString )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/XMLLookupService" );
    coral::IHandle<coral::ILookupService> lookupSvc =
      context.query<coral::ILookupService>( "CORAL/Services/XMLLookupService" );

    if ( ! lookupSvc.isValid() )
    {
      throw std::runtime_error( "Can't load Lookup Service" );
    }

    coral::IDatabaseServiceSet* dbSet = lookupSvc->lookup( connectionString );

    if( dbSet->numberOfReplicas() == 0 )
    {
      throw std::runtime_error( "No replicas found" );
    }

    return dbSet->replica( 0 );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  const coral::IAuthenticationCredentials&
  getCredentials( const coral::IDatabaseServiceDescription& svdesc )
  {
    // Get the global context
    coral::Context& context = coral::Context::instance();

    context.loadComponent( "CORAL/Services/XMLAuthenticationService" );
    coral::IHandle<coral::IAuthenticationService> authSvc =
      context.query<coral::IAuthenticationService>(  "CORAL/Services/XMLAuthenticationService" );

    if ( ! authSvc.isValid() )
    {
      throw std::runtime_error( "Can't load Authentication Service" );
    }

    return authSvc->credentials( svdesc.connectionString() );
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

} // empty namespace

namespace coral
{
  class AliasesSynonymsTest;
}

//----------------------------------------------------------------------------

class coral::AliasesSynonymsTest : public coral::CoralCppUnitDBTest
{
  CPPUNIT_TEST_SUITE( AliasesSynonymsTest );
  CPPUNIT_TEST( test_synonyms );
  CPPUNIT_TEST_SUITE_END();

public:

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  void test_synonyms()
  {

    //The string for the table name is created coding the input of the
    // slot and platform name
    std::string T1 = BuildUniqueTableName( "SYN_T1" );

    //The credential for the UPDATE connection
    const coral::IDatabaseServiceDescription& svdesc =
      getServiceDescription("CORAL-Oracle-lcgnight/admin");

    const coral::IAuthenticationCredentials& creds = getCredentials( svdesc );

    //=================================================================
    //
    // CREATION OF TABLE SECTION
    //
    //=================================================================

    {

      coral::ISession* session = getSession( svdesc, creds );

      session->transaction().start();

      coral::ISchema& schema = session->nominalSchema();

      //Create a new table with the selected name if it does not exist
      if( !schema.existsTable(T1) )
      {
        std::cout << "Table " << T1 << " was not created!!" << std::endl;

        coral::TableDescription description1( "Synonyms_Test" );

        description1.setName(T1);

        description1.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long) ) );

        schema.createTable( description1 );

        coral::sleepSeconds( 1 );

      }

      session->transaction().commit();
      delete session;
    }

    //=================================================================
    //
    // CREATION SYNONYM SECTION
    //
    //=================================================================

    //Create strings for SYNONYM, SQL COMMAND, SQLPLUS CONNECTION STRING
    std::stringstream synonym;
    synonym<<"SYNONYM_OF_"<<T1;

    //Create string for the sql command
    std::stringstream cmd;
    cmd << "sqlplus -L -S "
        <<creds.valueForItem( creds.userItem() )<<"/"
        <<creds.valueForItem( creds.passwordItem() )
        << "@lcg_coral_nightly <<ENDOFSQL \n"
        << "create or replace synonym "
        << synonym.str() << " for "
        << creds.valueForItem( creds.userItem() )<< "." << T1 << "; \n"
        << "exit; \n"
        << "ENDOFSQL";

    //Execute the SQLPLUS command and create SYNONYM of the table
    system(cmd.str().c_str());


    //=================================================================
    //
    // TEST TABLE SYNONYM SECTION
    //
    //=================================================================

    {

      coral::ISession* session = getSession( svdesc, creds );

      session->transaction().start();
      coral::ISchema& schema = session->nominalSchema();

      //Check if the TABLE and SYNONYM were properly created
      if( !schema.existsTable(T1) )
        throw std::runtime_error( "Table T1 does not exist" );

      if( !schema.existsTable(synonym.str()) )
        throw std::runtime_error( "Synonym does not exist or does not point to a existing table" );

      //Drop the table and the synonym if they exist
      schema.dropIfExistsTable( T1 );

      //If the drop of the synonym is executed, it does not work.
      //No error message appears for the method dropIfExistsTable or dropTable
      //but they results ineffective. This could be a BUG!!!!

      //if( schema.existsTable(synonym.str()) )
      //schema.dropTable( synonym.str() );

      session->transaction().commit();
      delete session;
    }

    //Create string for the sql command to drop SYNONYM
    std::stringstream cmd1;
    cmd1 << "sqlplus -L -S "
         <<creds.valueForItem( creds.userItem() )<<"/"
         <<creds.valueForItem( creds.passwordItem() )
         << "@lcg_coral_nightly <<ENDOFSQL \n"
         << "drop synonym " << synonym.str() << "; \n"
         << "exit; \n"
         << "ENDOFSQL";

    //Execute the SQLPLUS command and create SYNONYM of the table
    system(cmd1.str().c_str());
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  AliasesSynonymsTest(){}
  ~AliasesSynonymsTest(){}

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::AliasesSynonymsTest );

//----------------------------------------------------------------------------

CORALCPPUNITTEST_MAIN( AliasesSynonymsTest )
