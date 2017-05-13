// $Id: ThreadManager.cpp,v 1.9.2.3 2010/12/20 14:04:40 avalassi Exp $

#include "CoralServerBase/InternalErrorException.h"
#include "CoralBase/boost_thread_headers.h"
#include "ThreadManager.h"

using namespace coral::CoralSockets;

#include <iostream>
#define LOGGER_NAME "CoralSockets::ThreadManager"
#include "logger.h"
#include "StatsTypeThreads.h"

#undef DEBUG
#define DEBUG( out )

//----------------------------------------------------------------------------

ThreadManager::~ThreadManager()
{
  DEBUG( "ThreadManager::~ThreadManager called" );
  joinAll();
  cleanUp();
  DEBUG( "ThreadManager::~ThreadManager call finished" );
}

//----------------------------------------------------------------------------

void ThreadManager::addThread( Runnable *runClass )
{
  try {
    DEBUG( "ThreadManager::addThread("<<runClass<<") called" );
    boost::mutex::scoped_lock lock( m_threadsMutex );

    if ( m_threads.find( runClass) != m_threads.end() )
      throw GenericSocketException("can't add a class twice to a thread"
                                   " manager","ThreadManager::createThread");

    ThreadWrapper wrap( this, runClass );
    ThreadPtr tmp( new boost::thread( wrap ) );

    m_group.add_thread( tmp );
    m_threads[ runClass ] = tmp;
    m_threadsStatus[ runClass ] = tsStarted;
    DEBUG( "ThreadManager::addThread("<<runClass<<") finished" );
  }
  catch (std::exception &e) {
    ERROR("caught exception when creating thread: "<< e.what());
    throw;
  }
  catch (...) {
    ERROR( "caught unknown exception while creating thread.");
    throw;
  }
}

//----------------------------------------------------------------------------

bool ThreadManager::existsThread( Runnable *runClass )
{
  DEBUG( "ThreadManager::existsThread("<<runClass<<") called" );
  boost::mutex::scoped_lock lock( m_threadsMutex );
  bool ret=m_threads.find( runClass) != m_threads.end();
  return ret;
}

//----------------------------------------------------------------------------

void ThreadManager::markDoneThread( Runnable *runClass )
{
  DEBUG( "ThreadManager::markDoneThread("<< runClass<<") called" );
  if (!existsThread( runClass ) )
    throw GenericSocketException("runClass doesn't exist!",
                                 "ThreadManager::markDoneThread");

  boost::mutex::scoped_lock lock( m_threadsMutex );
  m_threadsStatus[ runClass ] = tsDone;
}

//----------------------------------------------------------------------------

void ThreadManager::housekeeping()
{
  cleanUp();
}

//----------------------------------------------------------------------------

void ThreadManager::cleanUp( ) {
  DEBUG("ThreadManager::cleanUp("<< call_join<<")");
  boost::mutex::scoped_lock lock( m_threadsMutex );

  std::map<Runnable*, ThreadState >::iterator p;
  for(p = m_threadsStatus.begin(); p!=m_threadsStatus.end(); )
  {
    if ( p->second == tsJoined || p->second == tsDone ) {
      // thread is done
      DEBUG( "thread "<< p->first <<" is done" );

      if ( p->second != tsJoined )
        m_threads[ p->first ]->join();
      m_group.remove_thread( m_threads[ p->first ] );
      delete m_threads[ p->first ];
      delete p->first;

      m_threads.erase( p->first );
      m_threadsStatus.erase( p++ );
    }
    else
      ++p;
  };
}

//----------------------------------------------------------------------------

void ThreadManager::join( Runnable *runClass )
{
  DEBUG( "ThreadManager::join("<<runClass<<") called" );

  ThreadPtr thread;
  {
    boost::mutex::scoped_lock lock( m_threadsMutex );

    if (m_threads.find( runClass) != m_threads.end() )
      throw GenericSocketException("runClass doesn't exit",
                                   "ThreadManager::join");

    thread = m_threads.find( runClass )->second;
  };
  thread->join();
  // theoretical race condition, if housekeeping() is called after join
  // has finished and before we can grab the mutex. In that case join
  // will be called twice.
  boost::mutex::scoped_lock lock( m_threadsMutex );
  if (m_threads.find( runClass) != m_threads.end() )
    throw GenericSocketException("PANIC! runClass doesn't exit",
                                 "ThreadManager::join");
  m_threadsStatus[ runClass ] = tsJoined;

}


//----------------------------------------------------------------------------

void ThreadManager::joinAll() {
  DEBUG( "ThreadManager::joinAll() called" );
  m_group.join_all();

  // there is a theoretical race condition, if someone adds a thread
  // right after join_all() finished and it ends before we can grab
  // the mutex. In that case it will be marked as joined even if it
  // isn't :-(

  boost::mutex::scoped_lock lock( m_threadsMutex );
  std::map<Runnable*, ThreadState >::iterator p;
  for(p = m_threadsStatus.begin(); p!=m_threadsStatus.end(); ++p)
  {
    if ( p->second == tsDone ) {
      // thread is done and joined
      DEBUG( "thread "<< p->first <<" is done" );
      p->second = tsJoined;
    }
  };

  DEBUG( "ThreadManager::joinAll() finished" );
}

//----------------------------------------------------------------------------

const std::vector<coral::IThreadMonitor::ThreadRecord>
ThreadManager::listThreads() const
{
  DEBUG( "ThreadManager:listThreads" );
  boost::mutex::scoped_lock lock( m_threadsMutex );

  std::vector<ThreadRecord> records;
  for ( std::map<Runnable*, ThreadState >::const_iterator
          p = m_threadsStatus.begin(); p != m_threadsStatus.end(); ++p )
  {
    // Add this thread
    ThreadRecord record;
    record.id = p->first;
    record.description = p->first->desc();
    switch ( p->second )
    {
    case tsStarted:
      record.state = "STARTED";
      break;
    case tsDone:
      record.state = "DONE";
      break;
    case tsJoined:
      record.state = "JOINED";
      break;
    case tsDeleted:
      record.state = "DELETED";
      break;
    default:
      throw InternalErrorException( "PANIC! Unknown thread state",
                                    "ThreadManager::listThreads()",
                                    "coral::CoralSockets" );
    }
    records.push_back( record );
    // Is this thread itself a thread monitor (eg SocketThread)?
    IThreadMonitor* mon = dynamic_cast<IThreadMonitor*>( p->first );
    if ( mon )
    {
      const std::vector<ThreadRecord> records2 = mon->listThreads();
      for ( std::vector<ThreadRecord>::const_iterator
              rec2 = records2.begin(); rec2 != records2.end(); rec2++ )
        records.push_back( *rec2 );
    }
  }
  return records;
}

//----------------------------------------------------------------------------
