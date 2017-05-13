#include "ThreadApp.h"
#include "TestEnv/TestingConfig.h"

#include "TableCreateThread.h"
#include "TableQueryThread.h"

#include <vector>

//-----------------------------------------------------------------------------

ThreadApp::ThreadApp( const TestEnv& env )
  : Testing(env)
{
}

//-----------------------------------------------------------------------------

ThreadApp::~ThreadApp()
{
}

//-----------------------------------------------------------------------------

void
ThreadApp::write()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  std::vector< TableCreateThread* > threadBodies;

  for ( int i = 0; i < 10; ++i )
    threadBodies.push_back( new TableCreateThread( schema, i, T1 ) );

  std::vector< boost::thread* > threads;
  for ( int i = 0; i < 10; ++i )
    threads.push_back( new boost::thread( *( threadBodies[i] ) ) );

  for ( int i = 0; i < 10; ++i )
    threads[i]->join();

  for ( int i = 0; i < 10; ++i ) {
    delete threads[i];
    delete threadBodies[i];
  }

  coral::sleepSeconds( 1 );

  session.transaction().commit();
}

void
ThreadApp::read()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start( true );

  coral::ISchema& schema = session.nominalSchema();

  std::vector< TableQueryThread* > threadBodies;

  for ( int i = 0; i < 10; ++i )
    threadBodies.push_back( new TableQueryThread( schema, i, T1 ) );

  std::vector< boost::thread* > threads;
  for ( int i = 0; i < 10; ++i )
    threads.push_back( new boost::thread( *( threadBodies[i] ) ) );

  for ( int i = 0; i < 10; ++i )
    threads[i]->join();

  for ( int i = 0; i < 10; ++i ) {
    delete threads[i];
    delete threadBodies[i];
  }

  session.transaction().commit();
}

//-----------------------------------------------------------------------------
