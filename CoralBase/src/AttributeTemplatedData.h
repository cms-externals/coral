#ifndef CORALBASE_ATTRIBUTETEMPLATEDDATA_H
#define CORALBASE_ATTRIBUTETEMPLATEDDATA_H

#include <ostream>
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include "AttributeData.h"
#include "isNaN.h"

namespace coral {

  /// Templated specialization of the AttributeData class

  template<typename T>
  class AttributeTemplatedData : virtual public AttributeData
  {
  protected:
    virtual ~AttributeTemplatedData() {}

  public:
    AttributeTemplatedData() : AttributeData(), m_data(), m_externalVariable( 0 ), m_isNull( false )
    {}

    void setNull( bool isVariableNull )
    {
      if ( m_sharedData ) {
        m_sharedData->setNull( isVariableNull );
      }
      else {
        m_isNull = isVariableNull;
      }
    }

    bool isNull() const
    {
      if ( m_sharedData ) {
        return m_sharedData->isNull();
      }
      else {
        return m_isNull;
      }
    }

    bool operator==( const AttributeData& rhs ) const
    {
      return ( *static_cast< const T* >( this->addressOfData() ) == *static_cast< const T* >( rhs.addressOfData() ) );
    }

    bool operator!=( const AttributeData& rhs ) const
    {
      return ( *static_cast< const T* >( this->addressOfData() ) != *static_cast< const T* >( rhs.addressOfData() ) );
    }


    void* addressOfData() {
      if ( m_sharedData ) return m_sharedData->addressOfData();
      else return ( m_externalVariable ? m_externalVariable : &m_data );
    }

    const void* addressOfData() const {
      if ( m_sharedData ) return m_sharedData->addressOfData();
      else return ( m_externalVariable ? m_externalVariable : &m_data );
    }

    void setValueFromAddress( const void* externalAddress ) {
      if ( m_sharedData ) m_sharedData->setValueFromAddress( externalAddress );
      else {
        if ( m_externalVariable ) {
          *m_externalVariable = *( static_cast< const T*>( externalAddress ) );
        }
        else {
          m_data = *( static_cast< const T*>( externalAddress ) );
        }
      }
    }

    void copyValueToAddress( void* externalAddress ) const {
      if ( m_sharedData ) m_sharedData->copyValueToAddress( externalAddress );
      else {
        if ( m_externalVariable ) {
          *( static_cast< T*>( externalAddress ) ) = *m_externalVariable;
        }
        else {
          *( static_cast< T*>( externalAddress ) ) = m_data;
        }
      }
    }

    int size() const {
      return sizeof(T);
    }

    void bind( void* externalAddress ) {
      if ( m_sharedData ) m_sharedData->bind( externalAddress );
      else
        m_externalVariable = static_cast< T* >( externalAddress );
    }

    std::ostream& toOutputStream( std::ostream& os ) const {
      if ( this->isNull() ) {
        os << "NULL";
        return os;
      }
      if ( m_sharedData ) {
        return m_sharedData->toOutputStream( os );
      }
      else {
        if ( m_externalVariable ) {
          os << *static_cast<T*>( m_externalVariable );
        }
        else
          os << m_data;
        return os;
      }
    }

    bool isValidData() const {
      return true;
    }

  private:
    /// The actual data
    T m_data;

    /// The externally bound address
    T*   m_externalVariable;

    /// The null-ness
    bool m_isNull;
  };



  template<> int
  AttributeTemplatedData<std::string>::size() const {
    if ( m_sharedData ) return m_sharedData->size();
    else if ( m_externalVariable ) return m_externalVariable->size();
    else return m_data.size();
  }


  template<> int
  AttributeTemplatedData<coral::Blob>::size() const {
    if ( m_sharedData ) return m_sharedData->size();
    else if ( m_externalVariable ) return m_externalVariable->size();
    else return m_data.size();
  }



  /// Specialization for the streaming of the bool
  template<> std::ostream&
  AttributeTemplatedData<bool>::toOutputStream( std::ostream& os ) const {
    if ( this->isNull() ) {
      os << "NULL";
      return os;
    }
    bool data = m_data;
    if ( m_sharedData ) {
      data = *static_cast<const bool*>( m_sharedData->addressOfData() );
    }
    else if ( m_externalVariable ) {
      data = *static_cast<const bool*>( m_externalVariable );
    }
    os << (  ( data ) ? "TRUE" : "FALSE" );
    return os;
  }


  /// Specialization for the streaming of the unsigned char
  template<> std::ostream&
  AttributeTemplatedData<unsigned char>::toOutputStream( std::ostream& os ) const {
    if ( this->isNull() ) {
      os << "NULL";
      return os;
    }
    unsigned int data = static_cast<unsigned int>( m_data );
    if ( m_sharedData ) {
      data = static_cast<unsigned int>( *static_cast<const unsigned char*>( m_sharedData->addressOfData() ) );
    }
    else if ( m_externalVariable ) {
      data = static_cast<unsigned int>( *static_cast<const unsigned char*>( m_externalVariable ) );
    }
    os << data;
    return os;
  }


  /// Specialization for the streaming of the date
  template<> std::ostream&
  AttributeTemplatedData<coral::Date>::toOutputStream( std::ostream& os ) const {
    if ( this->isNull() ) {
      os << "NULL";
      return os;
    }
    const coral::Date* data = &m_data;
    if ( m_sharedData ) {
      data = static_cast<const coral::Date*>( m_sharedData->addressOfData() );
    }
    else if ( m_externalVariable ) {
      data = static_cast<const coral::Date*>( m_externalVariable );
    }
    os << data->year() << "/";
    int month = data->month();
    if ( month < 10 ) os << "0";
    os << month << "/";
    int day = data->day();
    if ( day < 10 ) os << "0";
    os << day;
    return os;
  }


  /// Specialization for the streaming of the time stamp
  template<> std::ostream&
  AttributeTemplatedData<coral::TimeStamp>::toOutputStream( std::ostream& os ) const {
    if ( this->isNull() ) {
      os << "NULL";
      return os;
    }
    const coral::TimeStamp* data = &m_data;
    if ( m_sharedData ) {
      data = static_cast<const coral::TimeStamp*>( m_sharedData->addressOfData() );
    }
    else if ( m_externalVariable ) {
      data = static_cast<const coral::TimeStamp*>( m_externalVariable );
    }
    os << data->year() << "/";
    int month = data->month();
    if ( month < 10 ) os << "0";
    os << month << "/";
    int day = data->day();
    if ( day < 10 ) os << "0";
    os << day << " ";
    int hour = data->hour();
    if ( hour < 10 ) os << "0";
    os << hour << ":";
    int minute = data->minute();
    if ( minute < 10 ) os << "0";
    os << minute << ":";
    int second = data->second();
    if ( minute < 10 ) os << "0";
    os << second << ".";
    long ns = data->nanosecond();
    if ( ns == 0 ) os << "0";
    else {
      int scale = 100000000;
      while ( ns / scale == 0 ) {
        os << "0";
        scale /= 10;
      }
      while ( ns%1000 == 0 && ns > 0 ) ns /= 1000;
      os << ns;
    }
    return os;
  }


  /// Specialization for the streaming of the blob
  template<> std::ostream&
  AttributeTemplatedData<coral::Blob>::toOutputStream( std::ostream& os ) const {
    if ( this->isNull() ) {
      os << "NULL";
      return os;
    }
    const coral::Blob* blob = &m_data;
    if ( m_sharedData ) {
      blob = static_cast<const coral::Blob*>( m_sharedData->addressOfData() );
    }
    else if ( m_externalVariable ) {
      blob = static_cast<const coral::Blob*>( m_externalVariable );
    }

    os << blob->size() << "@" << blob->startingAddress();
    return os;
  }

  template<> bool
  AttributeTemplatedData<std::string>::isValidData() const {
    if ( m_sharedData ) return m_sharedData->isValidData();
    else if ( m_externalVariable ) return !m_externalVariable->empty();
    else return !m_data.empty();
  }

  template<> bool
  AttributeTemplatedData<float>::isValidData() const {
    if ( m_sharedData ) return m_sharedData->isValidData();
    else if ( m_externalVariable ) return !isNaN(*m_externalVariable);
    else return !isNaN(m_data);
  }

  template<> bool
  AttributeTemplatedData<double>::isValidData() const {
    if ( m_sharedData ) return m_sharedData->isValidData();
    else if ( m_externalVariable ) return !isNaN(*m_externalVariable);
    else return !isNaN(m_data);
  }

}

#endif
