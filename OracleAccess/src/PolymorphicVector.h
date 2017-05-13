#ifndef ORACLEACCESS_POLYMORPHICVECTOR_H
#define ORACLEACCESS_POLYMORPHICVECTOR_H

#include <vector>
#include <string>
#include "oci.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

namespace coral {

  namespace OracleAccess {

    /// Base class of a polymorphic vector

    class PolymorphicVector {
    public:
      /// Constructor
      explicit PolymorphicVector( size_t cacheSize ) :
        m_indicators( new sb2[ cacheSize] )
      {}

      /// Destructor
      virtual ~PolymorphicVector() { delete [] m_indicators; }

      /// Clears the data array
      virtual void clear() = 0;

      /// Adds a new element (by copying its value)
      virtual void push_back( const void* dataPointer,
                              sb2 indicatorValue ) = 0;

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

    template< class T >
    class PolymorphicTVector : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      explicit PolymorphicTVector( size_t cacheSize ) : PolymorphicVector( cacheSize ), m_data(), m_serverVersion( 0 ), m_cacheSize( cacheSize )
      {}

      /// Constructor
      PolymorphicTVector( size_t cacheSize,
                          int serverVersion ) :
        PolymorphicVector( cacheSize ), m_data(), m_serverVersion( serverVersion ), m_cacheSize( cacheSize )
      {
        m_data.reserve( m_cacheSize );
      }

      /// Destructor
      virtual ~PolymorphicTVector() {}

      /// Clears the data array
      void clear() { m_data.clear(); }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        m_data.push_back( *( static_cast< const T* >( dataPointer ) ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(T); }

      /// Returns the oracle type
      ub2 dty() const { if ( m_serverVersion > 9 ) return SQLT_BFLOAT; else return SQLT_FLT; };

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(T); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector< T > m_data;
      int m_serverVersion;
      size_t m_cacheSize;
    };

    /// Template specializations
    template<> ub2
    PolymorphicTVector<int>::dty() const { return SQLT_INT; }

    template<> ub2
    PolymorphicTVector<char>::dty() const { return SQLT_AFC; }

    template<> ub2
    PolymorphicTVector<unsigned char>::dty() const { return SQLT_UIN; }

    template<> ub2
    PolymorphicTVector<unsigned int>::dty() const { return SQLT_UIN; }

    template<> ub2
    PolymorphicTVector<short>::dty() const { return SQLT_INT; }

    template<> ub2
    PolymorphicTVector<unsigned short>::dty() const { return SQLT_UIN; }

    template<> ub2
    PolymorphicTVector<long>::dty() const { return SQLT_INT; }

    template<> ub2
    PolymorphicTVector<unsigned long>::dty() const { return SQLT_UIN; }

    template<> ub2
    PolymorphicTVector<double>::dty() const { if ( m_serverVersion > 9 ) return SQLT_BDOUBLE; else return SQLT_FLT; }

    template<> ub2
    PolymorphicTVector<long double>::dty() const { if ( m_serverVersion > 9 ) return SQLT_BDOUBLE; else return SQLT_FLT; }

    /// bool specialization
    template<>
    class PolymorphicTVector<bool> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      explicit PolymorphicTVector( size_t cacheSize ) : PolymorphicVector( cacheSize ), m_data(), m_cacheSize( cacheSize )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {}

      /// Clears the data array
      void clear() { m_data.clear(); }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        const bool& bdata = *static_cast< const bool* >( dataPointer );
        m_data.push_back( static_cast< int >( bdata ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(int); }

      /// Returns the oracle type
      ub2 dty() const {return SQLT_INT; };

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




    /// Date specialization
    template<>
    class PolymorphicTVector<coral::Date> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {}

      /// Clears the data array
      void clear() {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        OCIDate data;
        m_data.push_back( data );
        const coral::Date* inputDate = static_cast<const coral::Date*>( dataPointer );
        OCIDateSetDate( &( m_data.back() ),
                        static_cast<sb2>( inputDate->year() ),
                        static_cast<ub1>( inputDate->month() ),
                        static_cast<ub1>( inputDate->day() ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(OCIDate); }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_ODT; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(OCIDate); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<OCIDate> m_data;
      size_t m_cacheSize;
    };




    /// TimeStamp specialization
    template<>
    class PolymorphicTVector<coral::TimeStamp> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize, OCIEnv* ociEnvHandle, OCIError* ociErrorHandle ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize ),
        m_ociEnvHandle( ociEnvHandle ),
        m_ociErrorHandle( ociErrorHandle )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {
        this->clear();
      }

      /// Clears the data array
      void clear() {
        for ( std::vector<OCIDateTime*>::iterator iTime = m_data.begin();
              iTime != m_data.end(); ++iTime ) {
          OCIDescriptorFree( *iTime, OCI_DTYPE_TIMESTAMP );
        }
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );

        // Construct a time stamp descriptor
        void* temporaryPointer = 0;
        OCIDescriptorAlloc( m_ociEnvHandle,
                            &temporaryPointer, OCI_DTYPE_TIMESTAMP, 0, 0 );
        OCIDateTime* theTime = static_cast< OCIDateTime* >( temporaryPointer );

        // Assign the time
        const coral::TimeStamp* inputTime = static_cast<const coral::TimeStamp*>( dataPointer );
        OCIDateTimeConstruct( m_ociEnvHandle,
                              m_ociErrorHandle,
                              theTime,
                              static_cast<sb2>( inputTime->year() ),
                              static_cast<ub1>( inputTime->month() ),
                              static_cast<ub1>( inputTime->day() ),
                              static_cast<ub1>( inputTime->hour() ),
                              static_cast<ub1>( inputTime->minute() ),
                              static_cast<ub1>( inputTime->second() ),
                              static_cast<ub4>( inputTime->nanosecond() ),
                              0, 0 );
        m_data.push_back( theTime );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(OCIDateTime*); }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_TIMESTAMP; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(OCIDateTime*); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<OCIDateTime*> m_data;
      size_t m_cacheSize;
      OCIEnv* m_ociEnvHandle;
      OCIError* m_ociErrorHandle;
    };





    /// Blob specialization
    template<>
    class PolymorphicTVector<coral::Blob> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize, OCIEnv* ociEnvHandle,  OCISvcCtx* ociSvcCtxHandle, OCIError* ociErrorHandle ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize ),
        m_ociEnvHandle( ociEnvHandle ),
        m_ociSvcCtxHandle( ociSvcCtxHandle ),
        m_ociErrorHandle( ociErrorHandle )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {
        this->clear();
      }

      /// Clears the data array
      void clear() {
        for ( std::vector<OCILobLocator*>::iterator iData = m_data.begin();
              iData != m_data.end(); ++iData ) {
          OCILobFreeTemporary( m_ociSvcCtxHandle,
                               m_ociErrorHandle,
                               *iData );
          OCIDescriptorFree( *iData, OCI_DTYPE_LOB );
        }
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );

        // Construct a lob locator descriptor
        void* temporaryPointer = 0;
        OCIDescriptorAlloc( m_ociEnvHandle,
                            &temporaryPointer, OCI_DTYPE_LOB, 0, 0 );
        OCILobLocator* theData = static_cast< OCILobLocator* >( temporaryPointer );

        // Create the temporary lob
        OCILobCreateTemporary( m_ociSvcCtxHandle,
                               m_ociErrorHandle,
                               theData,
                               OCI_DEFAULT, OCI_DEFAULT, OCI_TEMP_BLOB,
                               FALSE,
                               OCI_DEFAULT );

        // Write the data
        const coral::Blob& inputData = *static_cast<const coral::Blob*>( dataPointer );
        ub4 amtp = inputData.size();
        OCILobWriteAppend( m_ociSvcCtxHandle,
                           m_ociErrorHandle,
                           theData, &amtp,
                           const_cast<void*>( inputData.startingAddress() ),
                           inputData.size(),
                           OCI_ONE_PIECE, 0, 0, 0, 0 );

        m_data.push_back( theData );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(OCILobLocator*); }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_BLOB; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(OCILobLocator*); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<OCILobLocator*> m_data;
      size_t m_cacheSize;
      OCIEnv* m_ociEnvHandle;
      OCISvcCtx* m_ociSvcCtxHandle;
      OCIError* m_ociErrorHandle;
    };







    /// Int64 specialization
    template<>
    class PolymorphicTVector<long long> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize,
                          OCIError* ociErrorHandle ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize ),
        m_ociErrorHandle( ociErrorHandle )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {}

      /// Clears the data array
      void clear() {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        OCINumber data;
        m_data.push_back( data );
        OCINumberFromInt( m_ociErrorHandle, dataPointer,
                          sizeof(long long),
                          OCI_NUMBER_SIGNED, &( m_data.back() ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(OCINumber); }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_VNU; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(OCINumber); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<OCINumber> m_data;
      size_t m_cacheSize;
      OCIError* m_ociErrorHandle;
    };





    /// UInt64 specialization
    template<>
    class PolymorphicTVector<unsigned long long> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize,
                          OCIError* ociErrorHandle ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize ),
        m_ociErrorHandle( ociErrorHandle )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {}

      /// Clears the data array
      void clear() {
        m_data.clear();
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        OCINumber data;
        m_data.push_back( data );
        OCINumberFromInt( m_ociErrorHandle, dataPointer,
                          sizeof(unsigned long long),
                          OCI_NUMBER_UNSIGNED, &( m_data.back() ) );
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &(m_data.front()); }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return sizeof(OCINumber); }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_VNU; }

      /// Returns the address of the lengths array
      ub2* lengthArray() const { return 0; }

      /// Returns the skip parameter for the data values
      ub4 dataValueSkip() const { return sizeof(OCINumber); }

      /// Returns the skip parameter for the data length values
      ub4 dataLengthSkip() const { return 0; }

    private:
      std::vector<OCINumber> m_data;
      size_t m_cacheSize;
      OCIError* m_ociErrorHandle;
    };




    /// String specialization
    template<>
    class PolymorphicTVector<std::string> : virtual public PolymorphicVector
    {
    public:
      /// Constructor
      PolymorphicTVector( size_t cacheSize ) :
        PolymorphicVector( cacheSize ),
        m_data(),
        m_cacheSize( cacheSize ),
        m_maxSize( 0 )
      {}

      /// Destructor
      virtual ~PolymorphicTVector() {}

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
      void clear() {
        m_data.clear();
        m_maxSize = 0;
      }

      /// Adds a new element (by copying its value)
      void push_back( const void* dataPointer,
                      sb2 indicatorValue ) {
        m_indicators[ m_data.size() ] = indicatorValue;
        if ( m_data.empty() ) m_data.reserve( m_cacheSize );
        const std::string currentData = *( static_cast< const std::string * >( dataPointer ) );
        m_data.push_back( currentData );
        if ( currentData.size() > m_maxSize ) m_maxSize = currentData.size();
      }

      /// Returns the starting address of an object in the array
      const void* startingAddress() const { return &m_data[0]; }

      /// Returns the size of an object in the array in bytes
      unsigned int sizeOfObject() const { return m_maxSize + 1; }

      /// Returns the oracle type
      ub2 dty() const { return SQLT_STR; }

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

  }

}

#endif
