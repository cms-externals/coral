#include "NetworkFailure.h"
#include "TestEnv/TestingConfig.h"

#include "StoneHandler.h"
#include "NetworkFailureThreadStone.h"
#include "NetworkFailureThread01.h"

#include <iostream>
#include <stdexcept>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("NETFAULT"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

//-----------------------------------------------------------------------------

NetworkFailure::NetworkFailure( const TestEnv& env )
  : Testing(env)
  , m_sh( new StoneHandler )
{
  coral::IConnectionService& cservice = m_sw.getConnectionService();

  cservice.configuration().setConnectionRetrialPeriod(0);
  cservice.configuration().setConnectionRetrialTimeOut(0);
  cservice.configuration().setConnectionTimeOut(10);

  m_sh->activate();
}

//-----------------------------------------------------------------------------

NetworkFailure::~NetworkFailure()
{
  delete m_sh;
}

//-----------------------------------------------------------------------------

void
NetworkFailure::setup()
{
#ifndef _WIN32
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  /* only create the table if not exists */
  /* creation takes a while for 200K rows */
  if( !schema.existsTable( T1 ) )
  {
    schema.dropIfExistsTable( T1 );

    coral::TableDescription description;

    description.setName( T1 );
    description.insertColumn( "ID", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
    description.setPrimaryKey( "ID" );

    description.insertColumn( "DATA", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );

    coral::ITable& table = schema.createTable( description );

    coral::AttributeList rowBuffer;
    table.dataEditor().rowBuffer( rowBuffer );

    for ( size_t i = 0; i < 200; ++i )
    {
      rowBuffer[0].data<long long>() = i;
      rowBuffer[1].data<long long>() = i;

      table.dataEditor().insertRow( rowBuffer );
    }

    coral::sleepSeconds( 1 );
  }

  session.transaction().commit();

#endif
}

//-----------------------------------------------------------------------------

void
NetworkFailure::test01()
{
  // This test works only for UNIX/LINUX
  if( m_connsvc )
  {
    coral::IConnectionServiceConfiguration& config = m_connsvc->configuration();
    config.setMissingConnectionExclusionTime(0);
  }

#ifndef _WIN32
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  m_sh->deactivate();


  LOG( "******** DISCONNECT ********");

  coral::sleepSeconds( 1 );


  //system ( "telnet localhost 45000" );


  coral::sleepSeconds( 5 );

  try
  {
    coral::ISchema& schema = session.nominalSchema();

    coral::AttributeList rowBuffer;
    rowBuffer.extend<long long>( "ID" );
    rowBuffer.extend<long long>( "DATA" );

    LOG( "Starting with fetching rows without pre-caching");

    coral::IQuery* query = schema.tableHandle( T1 ).newQuery();
    query->defineOutput( rowBuffer );
    query->addToOutputList( "ID" );
    query->addToOutputList( "DATA" );

    {
      // Try to get some data from disconnected client
      coral::ICursor& cursor = query->execute();
      while ( cursor.next() )
      {
        throw;
      }
    }
  }
  catch(...)
  {

  }

  coral::sleepSeconds( 1 );

  LOG( "******** CONNECT ********");
  // Activate the repeater
  //if( system ( "sudo /sbin/ip route del blackhole 137.138.80.41" ) != 0 )
  m_sh->activate();

  coral::sleepSeconds( 5 );


  try
  {

    coral::ISchema& schema = session.nominalSchema();

    coral::AttributeList rowBuffer;
    rowBuffer.extend<long long>( "ID" );
    rowBuffer.extend<long long>( "DATA" );

    LOG( "Starting with fetching rows without pre-caching");

    coral::IQuery* query = schema.tableHandle( T1 ).newQuery();
    query->defineOutput( rowBuffer );
    query->addToOutputList( "ID" );
    query->addToOutputList( "DATA" );

    {
      // Try to get some data from disconnected client
      coral::ICursor& cursor = query->execute();
      if ( !cursor.next() )
      {
        throw;
        //rowBuffer.toOutputStream( std::cout ) << std::endl;
      }
    }

    delete query;

  }
  catch(...)
  {
    throw;
  }





  session.transaction().commit();

#endif
}

//-----------------------------------------------------------------------------


void
NetworkFailure::test03()
{
  // This test works only for UNIX/LINUX
  if( m_connsvc )
  {
    coral::IConnectionServiceConfiguration& config = m_connsvc->configuration();
    config.setMissingConnectionExclusionTime(0);
  }

#ifndef _WIN32


  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );


  //system ( "telnet localhost 45000" );


  coral::sleepSeconds( 5 );


  {
    LOG( "******** TEST02 : Disconnection within the Cursor Loop ********");

    coral::ISchema& schema = session.nominalSchema();

    coral::AttributeList rowBuffer;
    rowBuffer.extend<long long>( "ID" );
    rowBuffer.extend<long long>( "DATA" );

    LOG( "Starting with fetching rows without pre-caching");

    coral::IQuery* query = schema.tableHandle( T1 ).newQuery();
    query->defineOutput( rowBuffer );
    query->addToOutputList( "ID" );
    query->addToOutputList( "DATA" );

    {
      //int counter = 0;
      // Try to get some data from disconnected client
      coral::ICursor& cursor = query->execute();
      m_sh->deactivate();
      coral::sleepSeconds( 10 );
      m_sh->activate();
      while ( cursor.next() )
      {
      }
    }
  }

  session.transaction().commit();

#endif
}

void
NetworkFailure::test04()
{
  // This test works only for UNIX/LINUX
  if( m_connsvc )
  {
    coral::IConnectionServiceConfiguration& config = m_connsvc->configuration();
    config.setMissingConnectionExclusionTime(0);
  }


}
