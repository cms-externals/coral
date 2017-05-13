#include <iostream>
#include <stdexcept>
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "TestEnv/TestingConfig.h"
#include "DeleteCascade.h"

#define LOG( msg ){ coral::MessageStream myMsg("DCASCADE"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

DeleteCascade::DeleteCascade( const TestEnv& env )
  : Testing(env)
{
}

//-----------------------------------------------------------------------------

DeleteCascade::~DeleteCascade()
{
}

//-----------------------------------------------------------------------------

void
#ifdef CORAL240DC
DeleteCascade::setup( bool cascade )
#else
  DeleteCascade::setup( bool /*cascade*/ )
#endif
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  // first table referenced by the foreign key
  coral::TableDescription desc01;
  desc01.setName(T1);

  // second table
  coral::TableDescription desc02;
  desc02.setName(T2);

  // third table
  coral::TableDescription desc03;
  desc03.setName(T3);

  session.nominalSchema().dropIfExistsTable( desc02.name() );
  session.nominalSchema().dropIfExistsTable( desc03.name() );
  session.nominalSchema().dropIfExistsTable( desc01.name() );

  // create schema
  desc01.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  desc01.insertColumn( "data", "string", 255, false );
  desc01.setPrimaryKey( "id" );
  coral::ITable& table01 = session.nominalSchema().createTable( desc01 );

  // create schema
  desc02.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  desc02.insertColumn( "idt1", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  desc02.setPrimaryKey( "id" );
#ifdef CORAL240DC
  desc02.createForeignKey( T1 + "_t2fk", "idt1", T1, "id", cascade );
#else
  desc02.createForeignKey( T1 + "_t2fk", "idt1", T1, "id" );
#endif
  coral::ITable& table02 = session.nominalSchema().createTable( desc02 );

  // create schema
  desc03.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  desc03.insertColumn( "idt1", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  desc03.setPrimaryKey( "id" );
  coral::ITable& table03 = session.nominalSchema().createTable( desc03 );

#ifdef CORAL240DC
  table03.schemaEditor().createForeignKey( T1 + "_t3fk", "idt1", T1, "id", cascade );
#else
  table03.schemaEditor().createForeignKey( T1 + "_t3fk", "idt1", T1, "id" );
#endif
  // fill with some data
  coral::AttributeList rowBuffer01;
  table01.dataEditor().rowBuffer( rowBuffer01 );

  for( size_t i = 1; i < 11; i++ )
  {
    rowBuffer01[0].data<long long>() = i;
    rowBuffer01[1].data<std::string>() = "hello world";

    table01.dataEditor().insertRow( rowBuffer01 );
  }

  coral::AttributeList rowBuffer02;
  table02.dataEditor().rowBuffer( rowBuffer02 );

  for( size_t i = 1; i < 40; i++ )
  {
    rowBuffer02[0].data<long long>() = i;
    rowBuffer02[1].data<long long>() = i / 4 + 1;

    table02.dataEditor().insertRow( rowBuffer02 );
  }

  coral::AttributeList rowBuffer03;
  table03.dataEditor().rowBuffer( rowBuffer03 );

  for( size_t i = 1; i < 40; i++ )
  {
    rowBuffer03[0].data<long long>() = i;
    rowBuffer03[1].data<long long>() = i / 4 + 1;

    table03.dataEditor().insertRow( rowBuffer03 );
  }

  session.transaction().commit();

  coral::sleepSeconds(1); // Avoid ORA-01466
}

//-----------------------------------------------------------------------------

void
DeleteCascade::test( bool cascade )
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ITable& table01 = session.nominalSchema().tableHandle( T1 );

  coral::ITableDataEditor& ed01 = table01.dataEditor();

  coral::AttributeList rowBuffer01;
  //  table01.dataEditor().rowBuffer( rowBuffer01 );

  if( cascade )
  {
    ed01.deleteRows( "id=4", rowBuffer01 );

    ed01.deleteRows( "id=6", rowBuffer01 );
  }
  else
  {
    try
    {
      //  rowBuffer01[0].data<long long>() = 4;
      ed01.deleteRows( "id=4", rowBuffer01 );

      //  rowBuffer01[0].data<long long>() = 6;
      ed01.deleteRows( "id=6", rowBuffer01 );
    }
    catch(...)
    {

    }
  }

  session.transaction().commit();
}

//-----------------------------------------------------------------------------
