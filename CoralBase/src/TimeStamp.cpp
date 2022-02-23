#include "CoralBase/TimeStamp.h"

const boost::posix_time::ptime& coral::getTimeStampEpoch()
{
  //std::cout << "getTimeStampEpoch() called" << std::endl;
  //std::cout << "In TimeStamp.cpp: &(Epoch) is " << &(coral::TimeStamp::Epoch) << std::endl;
  static const boost::posix_time::ptime s_epoch( boost::gregorian::date(1970, 1, 1 ) );
  //std::cout << "&(getTimeStampEpoch()) will give " << &(s_epoch) << std::endl;
  //std::cout << "getTimeStampEpoch() will give " << s_epoch << std::endl;
  return s_epoch;
}

const boost::posix_time::ptime& coral::TimeStamp::Epoch = coral::getTimeStampEpoch();

//default constructor
coral::TimeStamp::TimeStamp()
  : m_time( boost::posix_time::microsec_clock::universal_time() )
  , m_isLocal(false)
  , m_localTime( nullptr )
{
}

coral::TimeStamp::TimeStamp( int year, int month,  int day, int hour, int minute, int second, long nanosecond, bool isLocalTime /*=true*/ )
  : m_time( boost::gregorian::date( year, month, day ),
            boost::posix_time::time_duration( hour, minute, second, ( boost::posix_time::time_duration::num_fractional_digits() == 6 ? nanosecond/1000 : nanosecond ) ) )
  , m_isLocal( isLocalTime )
{
  if( m_isLocal )
    m_localTime = std::unique_ptr<boost::posix_time::ptime>(
                                                          new boost::posix_time::ptime( coral::time::local_adj::utc_to_local( m_time ) ) );
}

coral::TimeStamp::TimeStamp( const boost::posix_time::ptime& pT, bool isLocalTime )
  : m_time( pT ), m_isLocal(isLocalTime), m_localTime( nullptr )
{
  if( m_isLocal )
    m_localTime = std::unique_ptr<boost::posix_time::ptime>(
                                                          new boost::posix_time::ptime( coral::time::local_adj::utc_to_local( m_time ) ) );
}

coral::TimeStamp::TimeStamp( coral::TimeStamp::ValueType& nsecs )
  : m_time(), m_isLocal( false ), m_localTime( nullptr )
{
  // This contructs the timestamp only assuming UTC
  coral::TimeStamp::ValueType lSecs  = nsecs/1000000000; // extract seconds     part
  coral::TimeStamp::ValueType lNsecs = nsecs%1000000000; // extract nanoseconds part

  // Little precision check
  long int precision = boost::posix_time::time_duration::num_fractional_digits();

  boost::posix_time::time_duration since_epoch;

  if( precision > 6 )
    since_epoch = boost::posix_time::seconds( (long)lSecs ) + coral::time::nanoseconds( lNsecs );
  else
    since_epoch = boost::posix_time::seconds( (long)lSecs ) + boost::posix_time::microseconds( lNsecs/1000 );

  boost::posix_time::ptime time_since_epoch( boost::gregorian::date( 1970, 1, 1 ), since_epoch );

  m_time = time_since_epoch;
}

coral::TimeStamp::ValueType coral::TimeStamp::total_nanoseconds() const
{
  // This is always returned in UTC, keeps the former SEAL semantics
  coral::TimeStamp::ValueType nsecs = 0L;

  boost::posix_time::time_duration since_epoch = m_time - coral::TimeStamp::Epoch;
  nsecs = since_epoch.total_nanoseconds();

  return nsecs;
}

coral::TimeStamp coral::TimeStamp::now( bool isLocalTime /*=false*/ )
{
  boost::posix_time::ptime timeNow;

  if( isLocalTime )
  {
    timeNow = boost::posix_time::microsec_clock::local_time();
  }
  else
  {
    timeNow = boost::posix_time::microsec_clock::universal_time();
  }

  return coral::TimeStamp( timeNow, isLocalTime );
}

std::string coral::TimeStamp::toString() const
{
  std::stringstream ss;
  ss << day()  << "/" << month()  << "/" << year()   << " "
     << hour() << ":" << minute() << ":" << second() << "." << nanosecond();
  return ss.str();
}

coral::TimeStamp::TimeStamp( const boost::posix_time::ptime& pT )
  : m_time( pT )
  , m_isLocal(false) // We assume UTC here
{
}

coral::TimeStamp::~TimeStamp()
{
}

coral::TimeStamp::TimeStamp( const coral::TimeStamp& rhs ) :
  m_time( rhs.m_time ), m_isLocal( rhs.m_isLocal )
{
  if( m_isLocal )
  {
    m_localTime = std::unique_ptr<boost::posix_time::ptime>(
                                                          new boost::posix_time::ptime( coral::time::local_adj::utc_to_local( m_time ) ) );
  }
}

coral::TimeStamp&
coral::TimeStamp::operator=( const coral::TimeStamp& rhs )
{
  if( this != &rhs )
  {
    m_time      = rhs.m_time;
    m_isLocal   = rhs.m_isLocal;

    if( m_isLocal )
    {
      m_localTime = std::unique_ptr<boost::posix_time::ptime>(
                                                            new boost::posix_time::ptime( coral::time::local_adj::utc_to_local( m_time ) ) );
    }
  }

  return *this;
}

int
coral::TimeStamp::year() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->date().year() : m_time.date().year() ;
}

int
coral::TimeStamp::month() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->date().month() : m_time.date().month() ;
}

int
coral::TimeStamp::day() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->date().day() : m_time.date().day() ;
}

int
coral::TimeStamp::hour() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->time_of_day().hours() : m_time.time_of_day().hours();
}

int
coral::TimeStamp::minute() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->time_of_day().minutes() : m_time.time_of_day().minutes();
}

int
coral::TimeStamp::second() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->time_of_day().seconds() : m_time.time_of_day().seconds();
}

long
coral::TimeStamp::nanosecond() const
{
  // system independent (from BOOST docs): count*(time_duration_ticks_per_second / count_ticks_per_second)
  long ns = (long)(( m_isLocal && m_localTime.get() != 0 ) ? m_localTime->time_of_day().fractional_seconds() : m_time.time_of_day().fractional_seconds());
  return ( boost::posix_time::time_duration::num_fractional_digits() == 6 ? ns*1000 : ns );
}

const boost::posix_time::ptime&
coral::TimeStamp::time() const
{
  return ( m_isLocal && m_localTime.get() != 0 ) ? *m_localTime : m_time;
}

bool
coral::TimeStamp::operator==( const coral::TimeStamp& rhs ) const
{
  return ( this->m_time == rhs.m_time );
}

bool
coral::TimeStamp::operator!=( const coral::TimeStamp& rhs ) const
{
  return ( this->m_time != rhs.m_time );
}
