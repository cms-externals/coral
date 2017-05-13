// $Id: ByteBuffer.cpp,v 1.9.2.2 2011/12/10 22:51:04 avalassi Exp $

// Include files
#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <sstream>
#include "CoralBase/Exception.h"
#include "CoralServerBase/ByteBuffer.h"

// Logger
#define LOGGER_NAME "CoralServerBase::ByteBuffer"
#include "CoralServerBase/logger.h"

// Namespace
using namespace coral;

//-----------------------------------------------------------------------------

ByteBuffer::ByteBuffer()
  : m_maxSize( 0 )
  , m_usedSize( 0 )
  , m_data( (unsigned char*) ::malloc( m_maxSize ) )
{
  if ( !m_data )
  {
    std::stringstream msg;
    msg << "Failed to malloc " << m_maxSize << " bytes";
    logger << Error << msg.str() << endlog;
    throw Exception( msg.str(),
                     "ByteBuffer::ByteBuffer",
                     "coral::CoralServerBase" );
  }
}

//-----------------------------------------------------------------------------

ByteBuffer::ByteBuffer( size_t newSize )
  : m_maxSize( newSize )
  , m_usedSize( 0 )
  , m_data( (unsigned char*) ::malloc( m_maxSize ) )
{
  if ( !m_data )
  {
    std::stringstream msg;
    msg << "Failed to malloc " << m_maxSize << " bytes";
    logger << Error << msg.str() << endlog;
    throw Exception( msg.str(),
                     "ByteBuffer::ByteBuffer",
                     "coral::CoralServerBase" );
  }
}

//-----------------------------------------------------------------------------

ByteBuffer::~ByteBuffer()
{
  if ( m_data )
  {
    ::free( m_data );
  }
  else
  {
    logger << Error << "Nothing to free in ByteBuffer::~ByteBuffer" << endlog;
  }
}

//-----------------------------------------------------------------------------

void ByteBuffer::setUsedSize( size_t newUsedSize )
{
  if ( newUsedSize > maxSize() )
    throw Exception( "Attempt to set new usedSize > maxSize",
                     "ByteBuffer::setUsedSize",
                     "coral::CoralServerBase" );
  m_usedSize = newUsedSize;
}

//-----------------------------------------------------------------------------

void ByteBuffer::resize( size_t newMaxSize )
{
  if ( newMaxSize == maxSize() ) return;  // do nothing
  m_maxSize = newMaxSize;
  if ( m_usedSize > newMaxSize ) m_usedSize = newMaxSize;
  unsigned char* tmp = (unsigned char*)::realloc( m_data, newMaxSize );
  if ( tmp )
  {
    m_data = tmp;
  }
  else
  {
    std::stringstream msg;
    msg << "Failed to realloc " << m_maxSize << " bytes";
    logger << Error << msg.str() << endlog;
    throw Exception( msg.str(),
                     "ByteBuffer::ByteBuffer",
                     "coral::CoralServerBase" );
  }
}

//-----------------------------------------------------------------------------

std::ostream& ByteBuffer::print( std::ostream& out ) const
{
  // Header
  out << "ByteBuffer[ maxSize=" << m_maxSize
      << ", usedSize=" << m_usedSize << ", ";
  // ASCII dump
  out << "ascii='";
  for ( size_t i=0; i<m_usedSize; i++ )
  {
    unsigned char cTmp = *(m_data+i);
    if ( !isprint( cTmp ) ) out << ".";
    else out << cTmp;
  }
  out << "', ";
  // HEX dump
  out << "hex=0x";
  for ( size_t i=0; i<m_usedSize; i++ )
  {
    // NB Need __unsigned__ char conversion!
    unsigned char cTmp = *(m_data+i);
    // Fix bug #53719 - sprintf prints 3 characters: 2 plus the trailing '\0'
    // [but note that the return value of sprintf is 2 even if it prints 3!]
    char xTmp[3];
    sprintf( xTmp, "%02X", cTmp );
    out << xTmp;
  }
  // Footer
  out << " ]";
  return out;
}

//-----------------------------------------------------------------------------
