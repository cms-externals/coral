#include "PrefetchTest.h"
#include "TestEnv/TestingConfig.h"

#include <iostream>
#include <stdexcept>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("PREFETCH"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

//-----------------------------------------------------------------------------

PreFetch::PreFetch( const TestEnv& env )
  : Testing(env)
{
}

//-----------------------------------------------------------------------------

PreFetch::~PreFetch()
{
}

//-----------------------------------------------------------------------------

void
PreFetch::write()
{
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

    for ( size_t i = 0; i < 200000; ++i )
    {
      rowBuffer[0].data<long long>() = i;
      rowBuffer[1].data<long long>() = i;

      table.dataEditor().insertRow( rowBuffer );
    }

    coral::sleepSeconds( 1 );
  }

  session.transaction().commit();
}

//-----------------------------------------------------------------------------

void
PreFetch::read()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& schema = session.nominalSchema();

  coral::AttributeList rowBuffer;
  rowBuffer.extend<long long>( "ID" );
  rowBuffer.extend<long long>( "DATA" );

  LOG( "Starting with fetching rows without pre-caching");

  time_t tvalue = ::time(0);

  coral::IQuery* query = schema.tableHandle( T1 ).newQuery();
  query->defineOutput( rowBuffer );
  query->addToOutputList( "ID" );
  query->addToOutputList( "DATA" );

  {
    coral::ICursor& cursor = query->execute();
    while ( cursor.next() )
    {
      //rowBuffer.toOutputStream( std::cout ) << std::endl;
    }
  }

  delete query;

  LOG( "Fetching rows without pre-caching takes [" << ::time(0) - tvalue << "] seconds");

  LOG( "Starting with fetching rows with rows-caching[10000]");

  tvalue = ::time(0);

  query = schema.tableHandle( T1 ).newQuery();
  query->defineOutput( rowBuffer );
  query->addToOutputList( "ID" );
  query->addToOutputList( "DATA" );

  // Set the prefetch-caches for rows and memory
  query->setRowCacheSize( 10000 );
  query->setMemoryCacheSize( 0 );

  {
    coral::ICursor& cursor = query->execute();
    while ( cursor.next() )
    {
      //rowBuffer.toOutputStream( std::cout ) << std::endl;
    }
  }

  delete query;

  LOG( "Fetching rows with rows-caching takes [" << ::time(0) - tvalue << "] seconds");

  LOG( "Starting with fetching rows with memory-caching[1MB]");

  tvalue = ::time(0);

  query = schema.tableHandle( T1 ).newQuery();
  query->defineOutput( rowBuffer );
  query->addToOutputList( "ID" );
  query->addToOutputList( "DATA" );

  query->setRowCacheSize( 0 );
  query->setMemoryCacheSize( 1 );

  {
    coral::ICursor& cursor = query->execute();
    while ( cursor.next() )
    {
      //rowBuffer.toOutputStream( std::cout ) << std::endl;
    }
  }

  delete query;

  LOG( "Fetching rows with memory-caching takes [" << ::time(0) - tvalue << "] seconds");


  session.transaction().commit();
}

//-----------------------------------------------------------------------------
