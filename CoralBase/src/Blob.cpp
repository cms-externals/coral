#include "CoralBase/Blob.h"
#include "CoralBase/Exception.h"
#include <cstdlib>
#include <cstring>

coral::Blob::Blob() :
  m_size( 0 ),
  m_data( 0 )
{
}


coral::Blob::Blob( long initialSizeInBytes ) :
  m_size( initialSizeInBytes ),
  m_data( ::malloc( initialSizeInBytes ) )
{
  if ( (!m_data) && m_size>0 ) throw Exception( "::malloc failed", "Blob::Blob(size)", "coral::CoralBase" );
}


coral::Blob::~Blob()
{
  if ( m_data ) ::free( m_data );
}


const void*
coral::Blob::startingAddress() const
{
  return m_data;
}


void*
coral::Blob::startingAddress()
{
  return m_data;
}


long
coral::Blob::size() const
{
  return m_size;
}


void
coral::Blob::extend( long additionalSizeInBytes )
{
  m_data = ::realloc( m_data, m_size + additionalSizeInBytes );
  m_size += additionalSizeInBytes;
  if ( (!m_data) && m_size>0 ) throw Exception( "::realloc failed", "Blob::extend", "coral::CoralBase" );
}


void
coral::Blob::resize( long sizeInBytes )
{
  if ( sizeInBytes != m_size ) {
    m_data = ::realloc( m_data, sizeInBytes );
    m_size = sizeInBytes;
    if ( (!m_data) && m_size>0 ) throw Exception( "::realloc failed", "Blob::resize", "coral::CoralBase" );
  }
}


coral::Blob::Blob( const coral::Blob& rhs ) :
  m_size( rhs.m_size ),
  m_data( 0 )
{
  if ( m_size > 0 ) {
    m_data = ::malloc( m_size );
    if ( (!m_data) && m_size>0 ) throw Exception( "::malloc failed", "Blob::Blob(Blob&)", "coral::CoralBase" );
    m_data = ::memcpy( m_data, rhs.m_data, m_size );
  }
}


coral::Blob&
coral::Blob::operator=( const Blob& rhs )
{
  if ( m_size > 0 ) {
    if ( rhs.m_size == 0 ) {
      ::free( m_data );
      m_data = 0;
      m_size = 0;
    }
    else {
      m_data = ::realloc( m_data, rhs.m_size );
      m_size = rhs.m_size;
      if ( (!m_data) && m_size>0 ) throw Exception( "::realloc failed", "Blob::operator=", "coral::CoralBase" );
      ::memcpy( m_data, rhs.m_data, m_size );
    }
  }
  else {
    if ( rhs.m_size > 0 ) {
      m_data = ::malloc( rhs.m_size );
      m_size = rhs.m_size;
      if ( (!m_data) && m_size>0 ) throw Exception( "::malloc failed", "Blob::operator=", "coral::CoralBase" );
      ::memcpy( m_data, rhs.m_data, m_size );
    }
  }
  return *this;
}


coral::Blob&
coral::Blob::operator+=( const coral::Blob& rhs )
{
  long initialSize = m_size;
  this->extend( rhs.size() );
  ::memcpy( static_cast<char*>( m_data ) + initialSize, rhs.m_data, rhs.size() );
  return *this;
}


bool
coral::Blob::operator==( const coral::Blob& rhs ) const
{
  if ( m_size != rhs.m_size ) return false;
  if ( m_size == 0 ) return true;
  const unsigned char* thisData = static_cast<const unsigned char*>( m_data );
  const unsigned char* rhsData = static_cast<const unsigned char*>( rhs.m_data );
  for ( long i = 0; i < m_size; ++i, ++thisData, ++rhsData )
    if ( *thisData != *rhsData )
      return false;
  return true;
}
