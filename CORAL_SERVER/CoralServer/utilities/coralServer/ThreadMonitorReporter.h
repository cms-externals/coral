// $Id: ThreadMonitorReporter.h,v 1.1.2.3 2010/12/20 14:04:40 avalassi Exp $
#ifndef CORALSERVER_THREADMONITORREPORTER_H
#define CORALSERVER_THREADMONITORREPORTER_H

// Include files
#include <fstream>
#include <iostream>
#include <time.h> // For nanosleep
//#include <unistd.h> // For sleep (see CoralCommon/Utilities.h for WIN32)
#include "CoralBase/TimeStamp.h"
#include "CoralServerBase/IThreadMonitor.h"
#include "CoralBase/boost_thread_headers.h"

namespace coral
{

  namespace CoralServer
  {

    void milliSleep( unsigned long msec )
    {
      struct timespec req = {0, 0};
      req.tv_sec = (int)(msec/1000);
      req.tv_nsec = ( msec-(req.tv_sec*1000) ) * 1000000;
      ::nanosleep( &req, 0 );
    }

    /** @class ThreadMonitorReporter
     *
     *  Reporter of IThreadMonitor status.
     *
     *  @author Andrea Valassi
     *  @date   2009-07-15
     */

    class ThreadMonitorReporter
    {

    private:

      /// Typedef
      typedef IThreadMonitor::ThreadRecord ThreadRecord;

      /// Reporter thread
      class ReporterThread
      {
      public:
        ReporterThread( const ThreadMonitorReporter& reporter,
                        const unsigned int period )
          : m_reporter( reporter )
          , m_period( period )
          , m_nsFirst( 0 )
          , m_iReport( 0 ){}
        ~ReporterThread(){}
        void operator()()
        {
          static const TimeStamp::ValueType e9 = 1000000000;
          m_nsFirst = TimeStamp::now().total_nanoseconds();
          // NB Do not use ReporterThread::m_isActive: boost::mutex creates
          // a copy of the original ReporterThread using the copy constructor!
          while ( m_period>0 && m_reporter.isActive() )
          {
            m_reporter.report(); // This may itself contain a sleep
            while ( ( TimeStamp::now().total_nanoseconds() - m_nsFirst ) / e9
                    < static_cast<signed long long>(m_period * ( m_iReport + 1 )) )
            {
              if ( !m_reporter.isActive() ) break;
              milliSleep( 100 ); // Check every 0.1s
            }
            ++m_iReport;
          }
        }
      private:
        const ThreadMonitorReporter& m_reporter;
        const unsigned int m_period;
        TimeStamp::ValueType m_nsFirst;
        unsigned long m_iReport;
      };

    public:

      /// Constructor from an IThreadMonitor const reference.
      ThreadMonitorReporter( const IThreadMonitor& threadMon,
                             const unsigned int period )
        : m_threadMon( threadMon )
        , m_repThread( 0 )
        , m_thread( 0 )
      {
        m_repThread = new ReporterThread( *this, period );
        m_thread = new boost::thread( *m_repThread );
      }

      /// Destructor.
      virtual ~ThreadMonitorReporter()
      {
        m_isActive = false;
        //std::cout << "Join thread" << std::endl;
        m_thread->join();
        //std::cout << "Delete ReporterThread" << std::endl;
        delete m_thread;
        //std::cout << "Delete thread" << std::endl;
        delete m_repThread;
      }

      /// Is this reporter active?
      bool isActive() const
      {
        return m_isActive;
      }

      /// Report the IThreadMonitor state.
      void report() const
      {
        static std::string file = ::getenv( "CORALSERVER_MONFILE" )
          ? std::string( ::getenv("CORALSERVER_MONFILE") )
          : std::string( "/tmp/" ) + ::getenv( "USER" ) + "/csm.txt";
        static unsigned int iReport = 0;
        // Use with 'less +F' or 'tail -f'
        // Add N-recs.size() lines at the end to keep a constant page size
        //static std::ofstream out( file.c_str() );
        // Use with 'tail -F' (remove stderr "file truncated")
        // e.g. (tail -F -s 0.1 /tmp/avalassi/csm.txt > /dev/tty) >& /dev/null
        std::ofstream out( file.c_str() );
        // Use 'tail -F -s 0.1' to check every 100ms (default: 1s)
        // otherwise tail may not realise that the file has shrunk
        milliSleep( 100 ); // so that 'tail -F' thinks that the file has shrunk
        std::string now = to_simple_string( coral::TimeStamp().time() );
        out << "*** " << now.substr( 0, now.size()-3 )
            << ": coralServer IThreadMonitor report ["
            << ++iReport << "] "<< std::endl;
        const std::vector<ThreadRecord> recs = m_threadMon.listThreads();
        for ( std::vector<ThreadRecord>::const_iterator
                rec = recs.begin(); rec != recs.end(); rec++ )
        {
          out << "Thread @" << rec->id
              << " ('" << rec->description << "')"
              << ": " << rec->state << std::endl;
        }
      }

    private:

      /// Is the reporter active?
      bool m_isActive;

      /// The IThreadMonitor const reference.
      const IThreadMonitor& m_threadMon;

      /// The reporter thread;
      ReporterThread* m_repThread;

      /// The reporter thread;
      boost::thread* m_thread;

    };

  }

}
#endif // CORALSERVER_THREADMONITORREPORTER_H
