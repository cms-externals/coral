#ifndef ORACLEACCESS_BINDVARIABLEHOLDER_H
#define ORACLEACCESS_BINDVARIABLEHOLDER_H

#include <boost/shared_ptr.hpp>

#include "oci.h"

namespace coral
{

  class Blob;
  class Date;
  class TimeStamp;

  namespace OracleAccess
  {

    class SessionProperties;

    /**
     *  @class IBindVariableHolder BindVariableHolder.h
     *
     *  Interface for a class holding the bind variables of the types
     *  that cannot be passed directly to Oracle
     */
    class IBindVariableHolder
    {
    public:

      /// Virtual destructor
      virtual ~IBindVariableHolder() {}

      /// Returns the status of the binding
      virtual bool succeeded() const = 0;

      /// Returns the address to be bound
      virtual void* bindAddress() const = 0;

      /// Returns the size of the attribute
      virtual ub4 size() const = 0;

    };


    /**
     * @class NumberVariableHolder
     * Implementation ofthe IBindVariableHolder class for Number data
     */
    class NumberVariableHolder : virtual public IBindVariableHolder
    {
    public:

      /// Constructor
      NumberVariableHolder(  void* numberData,
                             bool isSigned,
                             boost::shared_ptr<const SessionProperties> properties,
                             ub2 numberSize );

      /// Destructor
      virtual ~NumberVariableHolder();

      /// Returns the status of the binding
      bool succeeded() const;

      /// Returns the address to be bound
      void* bindAddress() const;

      /// Returns the size of the attribute
      ub4 size() const;

    private:

      /// The structure to be bound
      OCINumber* m_number;

    };



    /**
     * @class DateVariableHolder
     * Implementation ofthe IBindVariableHolder class for Date data
     */
    class DateVariableHolder : virtual public IBindVariableHolder
    {
    public:

      /// Constructor
      DateVariableHolder( const coral::Date& inputDate );

      /// Destructor
      virtual ~DateVariableHolder();

      /// Returns the status of the binding
      bool succeeded() const;

      /// Returns the address to be bound
      void* bindAddress() const;

      /// Returns the size of the attribute
      ub4 size() const;

    private:

      /// The structure to be bound
      OCIDate* m_date;

    };


    /**
     * @class TimeVariableHolder
     * Implementation ofthe IBindVariableHolder class for TimeStamp data
     */
    class TimeVariableHolder : virtual public IBindVariableHolder
    {
    public:

      /// Constructor
      TimeVariableHolder( const coral::TimeStamp& inputTime,
                          boost::shared_ptr<const SessionProperties> properties );

      /// Destructor
      virtual ~TimeVariableHolder();

      /// Returns the status of the binding
      bool succeeded() const;

      /// Returns the address to be bound
      void* bindAddress() const;

      /// Returns the size of the attribute
      ub4 size() const;

    private:

      /// The structure to be bound
      OCIDateTime* m_time;

    };



    /**
     * @class BlobVariableHolder
     * Implementation of the IBindVariableHolder class for BLOB data
     */
    class BlobVariableHolder : virtual public IBindVariableHolder
    {
    public:

      /// Constructor
      BlobVariableHolder( const coral::Blob& inputData,
                          boost::shared_ptr<const SessionProperties> sessionProperties );

      /// Destructor
      virtual ~BlobVariableHolder();

      /// Returns the status of the binding
      bool succeeded() const;

      /// Returns the address to be bound
      void* bindAddress() const;

      /// Returns the size of the attribute
      ub4 size() const;

    private:

      /// The structure to be bound
      OCILobLocator* m_data;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

    };


  }
}


/// Inline methods
inline void*
coral::OracleAccess::NumberVariableHolder::bindAddress() const
{
  return m_number;
}


inline ub4
coral::OracleAccess::NumberVariableHolder::size() const
{
  return sizeof(OCINumber);
}


inline bool
coral::OracleAccess::NumberVariableHolder::succeeded() const
{
  return ( m_number != 0 ) ? true : false;
}


inline void*
coral::OracleAccess::DateVariableHolder::bindAddress() const
{
  return m_date;
}


inline ub4
coral::OracleAccess::DateVariableHolder::size() const
{
  return sizeof(OCIDate);
}


inline bool
coral::OracleAccess::DateVariableHolder::succeeded() const
{
  return ( m_date != 0 ) ? true : false;
}


inline void*
coral::OracleAccess::TimeVariableHolder::bindAddress() const
{
  return const_cast<void*>( static_cast<const void*>( &m_time ) );
}


inline ub4
coral::OracleAccess::TimeVariableHolder::size() const
{
  return sizeof(OCIDateTime*);
}


inline bool
coral::OracleAccess::TimeVariableHolder::succeeded() const
{
  return ( m_time != 0 ) ? true : false;
}


inline void*
coral::OracleAccess::BlobVariableHolder::bindAddress() const
{
  return const_cast<void*>( static_cast<const void*>( &m_data ) );
}


inline ub4
coral::OracleAccess::BlobVariableHolder::size() const
{
  return sizeof(OCILobLocator*);
}


inline bool
coral::OracleAccess::BlobVariableHolder::succeeded() const
{
  return ( m_data != 0 ) ? true : false;
}

#endif
