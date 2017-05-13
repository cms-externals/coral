#include "CoralBase/TimeStamp.h"
#include "CoralCommon/SimpleTimer.h"

namespace coral
{
  SimpleTimer::SimpleTimer()
    : m_start()
    , m_duration( boost::posix_time::microseconds(0) )
    , m_running( false )
  {
  }

  SimpleTimer::~SimpleTimer()
  {
  }

  void SimpleTimer::start()
  {
    if( ! m_running )
    {
      m_start   = boost::posix_time::microsec_clock::local_time();
      m_running = true;
    }
  }

  void SimpleTimer::restart()
  {
    if( ! m_running )
      start();
  }

  SimpleTimer::ValueType SimpleTimer::sample()
  {
    SimpleTimer::ValueType soFar = 0;


    if( m_running )
    {
      boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
      m_duration                   = now - m_start;
      soFar                        = m_duration.total_microseconds();
    }

    return soFar;
  }

  void SimpleTimer::stop()
  {
    if( m_running )
    {
      sample();
      m_running = false;
    }
  }

  SimpleTimer::ValueType SimpleTimer::total()
  {
    return m_duration.total_microseconds();
  }
}
