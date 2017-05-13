#include "NetworkFailureThread01.h"
#include "TestEnv/TestingConfig.h"
#include "TestEnv/SealSwitch.h"
#include "TestEnv/Testing.h"
#include "RelationalAccess/ISchema.h"

#include <stdexcept>
#include <sstream>
#include <memory>
#include <cmath>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("NETFAULT"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

NetworkFailureThread01::NetworkFailureThread01( const TestEnv& env )
  : Testing(env)
{
}

NetworkFailureThread01::~NetworkFailureThread01()
{
}

void NetworkFailureThread01::operator()()
{
  createSession(0, coral::ReadOnly);

  LOG( "***** Thread-01 started *****" );

  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );


  coral::sleepSeconds( 1 );

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
        LOG( "fetch row" );
        coral::sleepSeconds( 1 );

      }
    }
  }


  session.transaction().commit();

  coral::sleepSeconds( 5 );

  deleteSession();

}
