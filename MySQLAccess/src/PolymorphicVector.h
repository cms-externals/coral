// $Id: PolymorphicVector.h,v 1.5 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_POLYMORPHICVECTOR_H
#define MYSQLACCESS_POLYMORPHICVECTOR_H 1

#include "MySQL_headers.h"

#include <string>
#include <vector>

#include "CoralBase/Blob.h"

namespace coral
{
  namespace MySQLAccess
  {
    typedef short sb2;
    typedef unsigned short ub2;
    typedef int sb4;
    typedef unsigned int ub4;

    /// Base class of a polymorphic vector

    class PolymorphicVector
    {
    public:
      /// Constructor
      explicit PolymorphicVector( size_t cacheSize )
        : m_indicators( new sb2[ cacheSize] )
      {
      }

      /// Destructor
      virtual ~PolymorphicVector() { delete [] m_indicators; }

      /// Clears the data array
      virtual void clear() = 0;

      /// Adds a new element (by copying its value)
      virtual void push_back( const void* dataPointer, sb2 indicatorValue ) = 0;

      /// Returns the starting address of an object in the array
      virtual const void* startingAddress() const = 0;

      /// Returns the size of an object in the array in bytes
      virtual unsigned int sizeOfObject() const = 0;

      /// Returns the oracle type
      virtual ub2 dty() const = 0;

      /// Returns the address of the lengths array
      virtual ub2* lengthArray() const = 0;

      /// Returns the skip parameter for the data values
      virtual ub4 dataValueSkip() const = 0;

      /// Returns the skip parameter for the data length values
      virtual ub4 dataLengthSkip() const = 0;

      /// Returns the address of the indicators array
      sb2* indicatorArray() const { return m_indicators; }

    protected:
      sb2* m_indicators;
    };

    /// Vector of a concrete type

    template< class T > class PolymorphicTVector : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      explicit PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data()  , m_serverVersion( 0 ) , m_cacheSize( cacheSize ) { }

      /// Constructor
      PolymorphicTVector( size_t cacheSize, int serverVersion )
        : PolymorphicVector( cacheSize ) , m_data() , m_serverVersion( serverVersion ) , m_cacheSize( cacheSize )
      {
        m_data.reserve( m_cacheSize );
      }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Clears the data array
      void clear()
      {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        m_data.push_back( *( static_cast< const T* >( dataPointer ) ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const
      {
        return &(m_data.front());
      }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const
      {
        return sizeof(T);
      }

      /// Returns the oracle type
      ub2 dty() const
      {
        return FIELD_TYPE_FLOAT;
      };

      /// Returns the address of the lengths array
      ub2* lengthArray() const
      {
        return 0;
      }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const
      {
        return sizeof(T);
      }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const
      {
        return 0;
      }

    private:
      std::vector< T > m_data;
      int m_serverVersion;
      size_t m_cacheSize;
    };

    /// Template specializations
    template<> ub2 PolymorphicTVector<int>::dty() const { return FIELD_TYPE_LONG; }
    template<> ub2 PolymorphicTVector<char>::dty() const { return FIELD_TYPE_TINY; }
    template<> ub2 PolymorphicTVector<unsigned char>::dty() const { return FIELD_TYPE_TINY; }
    template<> ub2 PolymorphicTVector<unsigned int>::dty() const { return FIELD_TYPE_LONG; }
    template<> ub2 PolymorphicTVector<short>::dty() const { return FIELD_TYPE_SHORT; }
    template<> ub2 PolymorphicTVector<unsigned short>::dty() const { return FIELD_TYPE_SHORT; }
    template<> ub2 PolymorphicTVector<long>::dty() const { return FIELD_TYPE_LONG; }
    template<> ub2 PolymorphicTVector<unsigned long>::dty() const { return FIELD_TYPE_LONG; }
    template<> ub2 PolymorphicTVector<float>::dty() const { return FIELD_TYPE_FLOAT; };
    template<> ub2 PolymorphicTVector<double>::dty() const { return FIELD_TYPE_DOUBLE; };
    template<> ub2 PolymorphicTVector<long double>::dty() const { return FIELD_TYPE_DOUBLE; }; // FIXME - verify this works

    /// bool specialization
    template<> class PolymorphicTVector<bool> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      explicit PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data() , m_cacheSize( cacheSize ) { }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Clears the data array
      void clear() { m_data.clear(); }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;

        if ( m_data.empty() )
          m_data.reserve( m_cacheSize );

        const bool& bdata = *static_cast< const bool* >( dataPointer );
        m_data.push_back( static_cast< int >( bdata ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(int); }

      /// Returns the oracle type
      ub2 dty() const {return FIELD_TYPE_TINY; };

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(int); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector< int > m_data;
      size_t m_cacheSize;
    };

    /// Int64 specialization
    template<> class PolymorphicTVector<long long> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data() , m_cacheSize( cacheSize ) { }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Clears the data array
      void clear()
      {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;

        if ( m_data.empty() )
          m_data.reserve( m_cacheSize );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const
      {
        return sizeof(long long);
      }

      /// Returns the MySQL type
      ub2 dty() const { return FIELD_TYPE_LONGLONG; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const
      {
        return sizeof(long long);
      }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<long long> m_data;
      size_t m_cacheSize;
    };

    /// UInt64 specialization
    template<> class PolymorphicTVector<unsigned long long> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data() , m_cacheSize( cacheSize ) { }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Clears the data array
      void clear()
      {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;

        if ( m_data.empty() )
          m_data.reserve( m_cacheSize );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const
      {
        return sizeof(unsigned long long);
      }

      /// Returns the oracle type
      ub2 dty() const { return FIELD_TYPE_LONGLONG; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const
      {
        return sizeof(unsigned long long);
      }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<unsigned long long> m_data;
      size_t m_cacheSize;
    };

    /// String specialization
    template<> class PolymorphicTVector<std::string> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data() , m_cacheSize( cacheSize ) , m_maxSize( 0 ) { }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Returns the string in a given position
      const std::string& data( size_t i ) const
      {
        return m_data[i];
      }

      /// Returns the indicator in a given position
      sb2* indicatorData( size_t i ) const
      {
        return &m_indicators[i];
      }

      /// Clears the data array
      void clear()
      {
        m_data.clear();
        m_maxSize = 0;
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;

        if ( m_data.empty() )
          m_data.reserve( m_cacheSize );

        const std::string currentData = *( static_cast< const std::string * >( dataPointer ) );
        m_data.push_back( currentData );

        if ( currentData.size() > m_maxSize )
          m_maxSize = currentData.size();
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &m_data[0]; }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return m_maxSize + 1; }

      /// Returns the oracle type
      ub2 dty() const { return FIELD_TYPE_STRING; } // FIXME - verify it should not be VARSTRING instead

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return 0; }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<std::string> m_data;
      size_t m_cacheSize;
      size_t m_maxSize;
    };

    /// Blob specialization
    template<> class PolymorphicTVector<coral::Blob> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize )
        : PolymorphicVector( cacheSize ) , m_data() , m_cacheSize( cacheSize ) , m_maxSize( 0 ) { }

      /// Destructor
      virtual ~PolymorphicTVector() { }

      /// Returns the blob in a given position
      coral::Blob& data( size_t i )
      {
        return m_data[i];
      }

      /// Returns the indicator in a given position
      sb2* indicatorData( size_t i ) const
      {
        return &m_indicators[i];
      }

      /// Clears the data array
      void clear()
      {
        m_data.clear();
        m_maxSize = 0;
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer, sb2 indicatorValue )
      {
        m_indicators[ m_data.size() ] = indicatorValue;

        if ( m_data.empty() )
          m_data.reserve( m_cacheSize );

        const coral::Blob& currentData = *( static_cast< const coral::Blob * >( dataPointer ) );
        m_data.push_back( currentData );
        size_t currentSize = currentData.size();

        if ( currentSize > m_maxSize )
          m_maxSize = currentSize;
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &m_data[0]; }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return m_maxSize; }

      /// Returns the oracle type
      ub2 dty() const { return FIELD_TYPE_BLOB; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return 0; }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<coral::Blob> m_data;
      size_t m_cacheSize;
      size_t m_maxSize;
    };

  }

}

#endif
