#include "ChangeNotifications.h"
#include "TestEnv/TestingConfig.h"

#include <iostream>
#include <stdexcept>
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

#define LOG( msg ){ coral::MessageStream myMsg("CHANGENOTF"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

//-----------------------------------------------------------------------------
#ifdef CORAL240CN
void
Notification::event()
{
  LOG("********* Received Callback Event ************");
}
#endif
//-----------------------------------------------------------------------------

ChangeNotifications::ChangeNotifications( const TestEnv& env )
  : Testing(env)
{
}

//-----------------------------------------------------------------------------

ChangeNotifications::~ChangeNotifications()
{
}

//-----------------------------------------------------------------------------

void
ChangeNotifications::setup()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  schema.dropIfExistsTable( T1 );

  coral::TableDescription description;

  description.setName( T1 );
  description.insertColumn( "ID", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  description.setPrimaryKey( "ID" );

  description.insertColumn( "DATA", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );

  coral::ITable& table = schema.createTable( description );

  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );

  for ( size_t i = 0; i < 20; ++i )
  {
    rowBuffer[0].data<long long>() = i;
    rowBuffer[1].data<long long>() = i;

    table.dataEditor().insertRow( rowBuffer );
  }
  session.transaction().commit();

  coral::sleepSeconds( 1 );

#ifdef CORAL240CN
  schema.registerNotification(T1, m_notification);
#endif
}

//-----------------------------------------------------------------------------

void
ChangeNotifications::test01()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  coral::ITable& table = schema.tableHandle( T1 );

  coral::AttributeList rowBuffer;
  table.dataEditor().rowBuffer( rowBuffer );

  for ( size_t i = 21; i < 30; ++i )
  {
    rowBuffer[0].data<long long>() = i;
    rowBuffer[1].data<long long>() = i;

    table.dataEditor().insertRow( rowBuffer );
  }

  session.transaction().commit();

  coral::sleepSeconds( 1 );
}

//-----------------------------------------------------------------------------
