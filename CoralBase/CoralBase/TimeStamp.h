#ifndef CORAL_CORALBASE_TIMESTAMP_H
#define CORAL_CORALBASE_TIMESTAMP_H

#include "CoralBase/boost_datetime_headers.h"

namespace coral
{
  namespace time
  {
    //From BOOST: This local adjustor depends on the machine TZ settings-- highly dangerous!
    typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> local_adj;
    // In case it's missing in BOOST build - might fail to compile
    typedef boost::date_time::subsecond_duration<boost::posix_time::time_duration,1000000000> nanoseconds;
  }
}

namespace coral
{

  /**
     @class TimeStamp TimeStamp.h CoralBase/TimeStamp.h

     A class defining the TIMESTAMP type implemented on the boost::posix_time::ptime class
   */
  class TimeStamp
  {
  public:
  
    /// The 64 bit value for storing the raw time value as the count of nanoseconds since the epoch time UTC
    typedef signed long long int ValueType;
    
    /// WARNING! THIS IS OBSOLETE AND WILL BE REMOVED AT THE NEXT API CHANGE!
    /// Use coral::getTimeStampEpoch() instead of coral::TimeStamp:Epoch!
    static const boost::posix_time::ptime& Epoch;
    
  public:
    /// Default constructor
    TimeStamp();

    /// Constructor ( nanosecond argument need sto checked against the system supported precision, microsecond usually
    TimeStamp( int  year, int month,  int day,
               int  hour, int minute, int second, long nanosecond, bool isLocalTime = false );

    /// Constructor from a posix time
    explicit TimeStamp( const boost::posix_time::ptime& );

    /// Constructor from a posix time into local time possibly
    explicit TimeStamp( const boost::posix_time::ptime&, bool isLocalTime );
    
    /// Contructor from raw ValueType use on own risk when you know what you're doing
    explicit TimeStamp( ValueType& nsecs );

    /// Destructor
    ~TimeStamp();

    /// Copy constructor
    TimeStamp( const TimeStamp& rhs );

    /// Assignment operator
    TimeStamp& operator=( const TimeStamp& rhs );

    /// Returns the year
    int year() const;

    /// Returns the month [1-12]
    int month() const;

    /// Returns the day [1-31]
    int day() const;

    /// Returns the hour [0-23]
    int hour() const;

    /// Returns the minute [0-59]
    int minute() const;

    /// Returns the second [0-59]
    int second() const;

    /// Returns the nanosecond (depends on the system supported precision, usually microsecond precision) [0-999999(999)]
    long nanosecond() const;
    
    /// The number of nanoseconds from epoch 01/01/1970 UTC, normally should fit into 64bit signed integer, depends on the BOOST installation
    ValueType total_nanoseconds() const;

    /// Returns the underlying boost::posix_time::ptime object
    const boost::posix_time::ptime& time() const;

    /// Equal operator
    bool operator==( const TimeStamp& rhs ) const;

    /// Comparison operator
    bool operator!=( const TimeStamp& rhs ) const;

    /// Take the current snapshot in time
    static TimeStamp now( bool isLocalTime=false );
    
    /// Return the string representation of the time stamp
    std::string toString() const;

  private:
    /// The actual time object
    boost::posix_time::ptime m_time;
    /// Is the point in time UTC or a local time?
    bool                                    m_isLocal;
    /// In case a Timestamp is flagged as local time we initialize this one too
    std::auto_ptr<boost::posix_time::ptime> m_localTime;
  };

  /// The epoch constant timestamp used to calculate the total number of nanoseconds since the epoch time UTC
  const boost::posix_time::ptime& getTimeStampEpoch();
  
}
#endif


