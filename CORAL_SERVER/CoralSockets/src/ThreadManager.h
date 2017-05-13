// $Id: ThreadManager.h,v 1.10.2.3 2011/09/16 12:19:47 avalassi Exp $
#ifndef CORALSOCKETS_THREADMANAGER_H
#define CORALSOCKETS_THREADMANAGER_H 1

#include <map>
#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "CoralServerBase/IThreadMonitor.h"
#include "CoralBase/boost_thread_headers.h"
#include "CoralSockets/GenericSocketException.h"

namespace coral
{

  namespace CoralSockets
  {

    // Forward declaration
    class ThreadWrapper;

    class Runnable
    {
    public:

      /// signal the thread that it should end
      virtual void endThread() = 0;

      /// this method is the main method of the thread
      virtual void operator()() = 0;

      // return a description of the thread
      virtual const std::string desc() const = 0;

      // destructor
      virtual ~Runnable(){}
    };

    class ThreadManager : virtual public IThreadMonitor
    {

      typedef boost::thread* ThreadPtr;

    public:

      enum ThreadState {
        tsStarted,
        tsDone,
        tsJoined,
        tsDeleted
      };

      ThreadManager()
      {}

      virtual ~ThreadManager();

      /// takes the ownership of runClass
      void addThread( Runnable *runClass );

      /// returns true if the runClass is in the map
      bool existsThread( Runnable *runClass );

      /// called by the ThreadWrapper class after the
      /// thread is finished. runClass will be removed
      /// from the maps during the next call to housekeeping()
      void markDoneThread( Runnable *runClass );

      /// check which threads are done and remove them from the
      /// maps
      void housekeeping();

      /// waits until this thread has finished
      /// please avoid, as there is a small race condition inside
      /// (could be fixed with an additonal mutex)
      void join( Runnable *runClass );

      /// waits until all threads are finished
      void joinAll();

      /// List all existing threads.
      const std::vector<ThreadRecord> listThreads() const;

    private:

      /// copy constructor is private
      ThreadManager(const ThreadManager& );

      /// assignment operator as well
      ThreadManager& operator=(const ThreadManager& );

      /// joins done threads if requested and delets them
      void cleanUp();

      /// mutex protecting m_threads
      /// [NB: mutable because it can be locked by const methods too]
      mutable boost::mutex m_threadsMutex;

      /// a map of the running threads
      std::map<Runnable*, ThreadPtr > m_threads;

      /// which threads are done
      std::map<Runnable*, ThreadState > m_threadsStatus;

      /// the thread group
      boost::thread_group m_group;

    };

    // a wrapper to ensure that the threads a deregistered when they have
    // finished
    class ThreadWrapper
    {

    public:

      ThreadWrapper( ThreadManager * manager, Runnable* runClass)
        : m_manager( manager )
        , m_runClass( runClass )
      {}

      virtual ~ThreadWrapper()
      {}

      void operator()() {
        try {
          m_runClass->operator()();

          // avoid race conditions by making sure this thread has been
          // added before we remove it from the map
          while ( !m_manager->existsThread( m_runClass ) )
            usleep(10000);

          m_manager->markDoneThread( m_runClass );
        }
        catch (std::exception &e) {
          std::cerr << "PANIC! Uncaught exception '"<< e.what()
                    <<"' in thread. Ending thread gracefully." << std::endl;
        }
        catch (...) {
          std::cerr << "PANIC! Uncaught unknown exception in thread!"
            " Ending thread gracefully." << std::endl;
        }
      }

    private:

      ThreadManager* m_manager;
      Runnable* m_runClass;

    };

  }

}


#endif // CORALSOCKETS_THREADMANAGER_H
