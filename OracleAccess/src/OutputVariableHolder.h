#ifndef ORACLEACCESS_OUTPUTVARIABLEHOLDER_H
#define ORACLEACCESS_OUTPUTVARIABLEHOLDER_H 1

#include <string>
#include <boost/shared_ptr.hpp>

#include "oci.h"

#include "SessionProperties.h"

namespace coral
{

  class Blob;
  class Date;
  class TimeStamp;

  namespace OracleAccess
  {

    /**
     * @class IOutputVariableHolder OutputVariableHolder.h
     * Interface for a class holding the define variables of the types
     * that cannot be passed directly to Oracle
     */
    class IOutputVariableHolder
    {
    public:
      /// Destructor
      virtual ~IOutputVariableHolder() {}

      /// Checks if the setup succeeded.
      virtual bool suceeded() const = 0;

      /// Returns the transfer protocol
      virtual ub2 dty() const = 0;

      /// Returns the define address
      virtual void* defineAddress() const = 0;

      /// Returns the define size
      virtual ub2 size() const = 0;

      /// Returns the length pointer
      virtual ub2* lengthPointer() const { return 0; }

      /// Copies the data
      virtual bool copyData() = 0;
    };


    /// Implementation of the IOutputVariableHolder for primitive types
    template<typename T> class OutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      OutputVariableHolder( void* userData,
                            boost::shared_ptr<const SessionProperties> sessionProperties ) :
        m_data( *static_cast<T*>( userData ) ),
        m_sessionProperties( sessionProperties )
      {}

      /// Destructor
      virtual ~OutputVariableHolder() {}

      /// Checks if the setup succeeded.
      bool suceeded() const { return true; }

      /// Returns the transfer protocol
      ub2 dty() const { return SQLT_INT; };

      /// Returns the define address
      void* defineAddress() const { return const_cast<void*>( static_cast<const void*>( &m_data ) ); }

      /// Returns the define size
      ub2 size() const {return sizeof(T); }

      /// Copies the data
      bool copyData() { return true; }

    private:
      T& m_data;
      boost::shared_ptr<const SessionProperties> m_sessionProperties;
    };

    template<> inline ub2 OutputVariableHolder<float>::dty() const { return (m_sessionProperties->serverVersion() > 9) ? SQLT_BFLOAT : SQLT_FLT; }
    template<> inline ub2 OutputVariableHolder<double>::dty() const { return (m_sessionProperties->serverVersion() > 9) ? SQLT_BDOUBLE : SQLT_FLT; }
    template<> inline ub2 OutputVariableHolder<long double>::dty() const { return (m_sessionProperties->serverVersion() > 9) ? SQLT_BDOUBLE : SQLT_FLT; }
    template<> inline ub2 OutputVariableHolder<unsigned int>::dty() const { return SQLT_UIN; }
    template<> inline ub2 OutputVariableHolder<unsigned short>::dty() const { return SQLT_UIN; }
    template<> inline ub2 OutputVariableHolder<unsigned long>::dty() const { return SQLT_UIN; }
    template<> inline ub2 OutputVariableHolder<unsigned char>::dty() const { return SQLT_UIN; }
    template<> inline ub2 OutputVariableHolder<char>::dty() const { return SQLT_AFC; }


    /// Implementation of the IOutputVariableHolder for Date variables
    class DateOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      explicit DateOutputVariableHolder( coral::Date& date );

      /// Destructor
      virtual ~DateOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      coral::Date& m_date;

      /// The defined output
      OCIDate* m_ociDate;
    };


    /// Implementation of the IOutputVariableHolder for TimeStamp variables
    class TimeOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      TimeOutputVariableHolder( coral::TimeStamp& timeStamp,
                                boost::shared_ptr<const SessionProperties> sessionProperties );

      /// Destructor
      virtual ~TimeOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      coral::TimeStamp& m_time;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The defined output
      OCIDateTime* m_ociDateTime;
    };


    /// Implementation of the IOutputVariableHolder for 64bit integer variables
    class NumberOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      NumberOutputVariableHolder( void* userNumber,
                                  boost::shared_ptr<const SessionProperties> sessionProperties,
                                  bool isSigned );

      /// Destructor
      virtual ~NumberOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      void* m_userNumber;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The defined output
      OCINumber* m_ociNumber;

      /// The sign flag
      uword m_signFlag;
    };


    /// Implementation of the IOutputVariableHolder for Blob variables
    class BlobOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      BlobOutputVariableHolder( coral::Blob& blob,
                                boost::shared_ptr<const SessionProperties> sessionProperties );

      /// Destructor
      virtual ~BlobOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      coral::Blob& m_blob;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The LOB locator
      OCILobLocator* m_locator;
    };


    /// Implementation of the IOutputVariableHolder for CLOB variables
    class ClobOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      ClobOutputVariableHolder( std::string& userData,
                                boost::shared_ptr<const SessionProperties> sessionProperties );

      /// Destructor
      virtual ~ClobOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      std::string& m_userData;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The LOB locator
      OCILobLocator* m_locator;
    };


    /// Implementation of the IOutputVariableHolder for normal string variables
    class StringOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      StringOutputVariableHolder( std::string& userData,
                                  ub2 stringSize );

      /// Destructor
      virtual ~StringOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

    private:
      /// The user data
      std::string& m_userData;

      /// The C-array
      char* m_data;

      /// The size of the string
      ub2 m_size;
    };


    /// Implementation of the IOutputVariableHolder for LONG string variables
    class LongStringOutputVariableHolder : virtual public IOutputVariableHolder
    {
    public:
      /// Constructor
      explicit LongStringOutputVariableHolder( std::string& userData );

      /// Destructor
      virtual ~LongStringOutputVariableHolder();

      /// Checks if the setup succeeded.
      bool suceeded() const;

      /// Returns the transfer protocol
      ub2 dty() const;

      /// Returns the define address
      void* defineAddress() const;

      /// Returns the define size
      ub2 size() const;

      /// Copies the data
      bool copyData();

      /// Returns the length pointer
      ub2* lengthPointer() const;

    private:
      /// The user data
      std::string& m_userData;

      /// The C-array
      char* m_data;

      /// The size of the string
      ub2 m_size;
    };

  }
}


// Inline methods
inline bool
coral::OracleAccess::DateOutputVariableHolder::suceeded() const
{
  return ( m_ociDate != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::DateOutputVariableHolder::dty() const
{
  return SQLT_ODT;
}

inline void*
coral::OracleAccess::DateOutputVariableHolder::defineAddress() const
{
  return m_ociDate;
}

inline ub2
coral::OracleAccess::DateOutputVariableHolder::size() const
{
  return sizeof(OCIDate);
}


inline bool
coral::OracleAccess::TimeOutputVariableHolder::suceeded() const
{
  return ( m_ociDateTime != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::TimeOutputVariableHolder::dty() const
{
  return SQLT_TIMESTAMP;
}

inline void*
coral::OracleAccess::TimeOutputVariableHolder::defineAddress() const
{
  return const_cast<void*>( static_cast<const void*>( &m_ociDateTime ) );
}

inline ub2
coral::OracleAccess::TimeOutputVariableHolder::size() const
{
  return sizeof(OCIDateTime*);
}


inline bool
coral::OracleAccess::NumberOutputVariableHolder::suceeded() const
{
  return ( m_ociNumber != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::NumberOutputVariableHolder::dty() const
{
  return SQLT_VNU;
}

inline void*
coral::OracleAccess::NumberOutputVariableHolder::defineAddress() const
{
  return m_ociNumber;
}

inline ub2
coral::OracleAccess::NumberOutputVariableHolder::size() const
{
  return sizeof(OCINumber);
}


inline bool
coral::OracleAccess::BlobOutputVariableHolder::suceeded() const
{
  return ( m_locator != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::BlobOutputVariableHolder::dty() const
{
  return SQLT_BLOB;
}

inline void*
coral::OracleAccess::BlobOutputVariableHolder::defineAddress() const
{
  return const_cast<void*>(static_cast<const void*>( &m_locator ) );
}

inline ub2
coral::OracleAccess::BlobOutputVariableHolder::size() const
{
  return 0;
}


inline bool
coral::OracleAccess::ClobOutputVariableHolder::suceeded() const
{
  return ( m_locator != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::ClobOutputVariableHolder::dty() const
{
  return SQLT_CLOB;
}

inline void*
coral::OracleAccess::ClobOutputVariableHolder::defineAddress() const
{
  return const_cast<void*>(static_cast<const void*>( &m_locator ) );
}

inline ub2
coral::OracleAccess::ClobOutputVariableHolder::size() const
{
  return 0;
}


inline bool
coral::OracleAccess::StringOutputVariableHolder::suceeded() const
{
  return ( m_data != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::StringOutputVariableHolder::dty() const
{
  return SQLT_STR;
}

inline void*
coral::OracleAccess::StringOutputVariableHolder::defineAddress() const
{
  return m_data;
}

inline ub2
coral::OracleAccess::StringOutputVariableHolder::size() const
{
  return m_size;
}


inline bool
coral::OracleAccess::LongStringOutputVariableHolder::suceeded() const
{
  return ( m_data != 0 ) ? true : false;
}

inline ub2
coral::OracleAccess::LongStringOutputVariableHolder::dty() const
{
  return SQLT_LNG;
}

inline void*
coral::OracleAccess::LongStringOutputVariableHolder::defineAddress() const
{
  return m_data;
}

inline ub2
coral::OracleAccess::LongStringOutputVariableHolder::size() const
{
  return m_size;
}

inline ub2*
coral::OracleAccess::LongStringOutputVariableHolder::lengthPointer() const
{
  return const_cast<ub2*>( &m_size );
}

#endif
