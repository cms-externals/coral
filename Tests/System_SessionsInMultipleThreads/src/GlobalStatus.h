#ifndef GLOBALSTATUS_H
#define GLOBALSTATUS_H

#include "CoralBase/boost_thread_headers.h"

class GlobalStatus {
public:
  GlobalStatus() :
    m_mutex(),
    m_ok( true )
  {}

  void setFalse()
  {
    boost::mutex::scoped_lock lock( m_mutex );
    m_ok = false;
  }

  bool isOk() {
    boost::mutex::scoped_lock lock( m_mutex );
    return m_ok;
  }

private:
  boost::mutex m_mutex;
  bool m_ok;
};

#endif
