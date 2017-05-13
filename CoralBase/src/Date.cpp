#include "CoralBase/Date.h"

namespace coral {

  //default constructor
  Date::Date()
    : m_time( boost::posix_time::microsec_clock::universal_time() ) /*initial with current time*/
  {
  }

  //constructor taken year month and day
  Date::Date( int year, int month, int day )
    : m_time( boost::gregorian::date( year, month, day), boost::posix_time::time_duration(0, 0, 0, 0) )
  {
  }

  Date::~Date()
  {
  }

  Date
  Date::today()
  {
    boost::posix_time::ptime timeToday = boost::posix_time::microsec_clock::local_time();
    return Date( timeToday );
  }

}
