// $Id: RingBufferSocket.cpp,v 1.8.2.4 2011/09/16 14:37:55 avalassi Exp $

// Include files
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "RingBufferSocket.h"

// Logger
#define LOGGER_NAME "CoralSockets::RingBufferSocket"
#include "logger.h"

// Debug
#undef DEBUG
#define DEBUG( out )

// Namespace
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

RingBuffer::RingBuffer( size_t size )
  : m_size( size )
  , m_buffer( new unsigned char[m_size] )
  , m_start( 0 )
  , m_end( 0 )
  , m_isClosed( false )
{
  DEBUG("RingBuffer::RingBuffer constructor");
}

//-----------------------------------------------------------------------------

RingBuffer::~RingBuffer()
{
  DEBUG("RingBuffer::RingBuffer destructor");
  delete[] m_buffer;
}

//-----------------------------------------------------------------------------

int RingBuffer::read( unsigned char* buf, size_t len )
{
  if ( m_isClosed && usedSize()==0 )
    return 0;
  // empty
  if ( m_start == m_end )
    return -1;
  size_t read=0;
  // if the end is wrapped around, first copy everything
  // until the end
  if ( m_start > m_end )
  {
    size_t cpyLen=len;
    if ( cpyLen > (m_size - m_start ) ) cpyLen = m_size - m_start;
    DEBUG( "read with wrapping cpyLen " << cpyLen
           << " start " << m_start << " end " << m_end );
    ::memcpy( buf, &m_buffer[ m_start ], cpyLen );
    m_start=(m_start + cpyLen) % m_size;
    buf+=cpyLen;
    len-=cpyLen;
    read += cpyLen;
  }
  // the trivial case, just copy what we have, no wrapping around
  if ( len > 0 &&  m_start < m_end )
  {
    if ( len > (m_end - m_start) ) len = m_end - m_start;
    DEBUG( "read len " << len << "  start " << m_start << " end " << m_end );
    ::memcpy( buf, &m_buffer[ m_start ], len );
    m_start+=len;
    read+=len;
  }
  return read;
}

//-----------------------------------------------------------------------------

int RingBuffer::write( unsigned char* buf, size_t len )
{
  if ( m_isClosed )
    return 0;
  if (len > freeSize() )
    len = freeSize();
  size_t written=0;
  // we have to wrap around
  if ( m_start <= m_end )
  {
    size_t cpyLen = len;
    if ( cpyLen > (m_size- m_end ) ) cpyLen = m_size - m_end;
    DEBUG( "write with wrapping cpyLen " << cpyLen
           << " start " << m_start << " end " << m_end );
    ::memcpy( &m_buffer[ m_end ], buf, cpyLen );
    m_end=(m_end+cpyLen) % m_size;
    buf+=cpyLen;
    written+=cpyLen;
    len-=cpyLen;
  }
  if ( len > 0 && m_start > m_end )
  {
    if ( len > (m_start - m_end -1 ) ) len = m_start - m_end -1;
    ::memcpy( &m_buffer[ m_end ], buf, len );
    DEBUG( "write len " << len << "  start " << m_start << " end " << m_end );
    m_end+=len;
    written+=len;
  }
  if ( written == 0 ) return -1;
  DEBUG("end: " << m_end);
  return written;
}

//-----------------------------------------------------------------------------

size_t RingBuffer::usedSize()
{
  return (m_size - m_start + m_end) % m_size;
}

//----------------------------------------------------------------------

bool RingBufferSocket::poll( ISocket::PMode mode, int timeout )
{
  if ( mode != P_READ && mode != P_WRITE )
    throw GenericSocketException("Panic! Wrong mode in poll!");
  int step = timeout>0 ? timeout/10 : 5;
  int sleep = timeout;
  if ( mode == P_READ )
  {
    while ( sleep > 0 || timeout<0  )
    {
      {
        boost::mutex::scoped_lock lock(m_readMutex);
        if ( m_read.usedSize() > 0 ) return true;
      }
      usleep( step*1000 );
      sleep-= step;
    }
    boost::mutex::scoped_lock lock(m_readMutex);
    return m_read.usedSize() > 0;
  }
  else
  {
    while ( sleep > 0 || timeout<0 )
    {
      {
        boost::mutex::scoped_lock lock(m_writeMutex);
        if ( m_write.freeSize() > 0 ) return true;
      }
      usleep( step*1000 );
      sleep-=step;
    }
    boost::mutex::scoped_lock lock(m_writeMutex);
    return m_write.freeSize() > 0;
  }
}

//----------------------------------------------------------------------

void RingBufferSocket::readAll( unsigned char* buf, size_t len )
{
  size_t res = read_write( P_READ, buf, len, -1 );
  if ( res != len )
    throw GenericSocketException("Panic! readAll() could not read all bytes!");
}

//----------------------------------------------------------------------

void RingBufferSocket::writeAll( const unsigned char* buf, size_t len )
{
  size_t res = read_write( P_WRITE, const_cast<unsigned char*>(buf), len, -1 );
  if ( res != len )
    throw GenericSocketException("Panic! writedAll() could not write all bytes!");
}

//----------------------------------------------------------------------

size_t RingBufferSocket::read( unsigned char* buf, size_t len, int timeout )
{
  return read_write( P_READ, buf, len, timeout );
}

//----------------------------------------------------------------------

size_t RingBufferSocket::write( const unsigned char* buf,
                                size_t len,
                                int timeout )
{
  return read_write( P_WRITE, const_cast<unsigned char*>(buf), len, timeout );
}

//----------------------------------------------------------------------

size_t RingBufferSocket::read_write( PMode mode,
                                     unsigned char* buf,
                                     size_t len,
                                     int timeout )
{
  if ( mode != P_READ && mode != P_WRITE )
    throw GenericSocketException("Panic! Wrong mode in read_write!");
  const char* label = ( mode==P_READ
                        ? "RingBufferSocket::read"
                        : "RingBufferSocket::write" );
  DEBUG( label << ":" << m_desc << " enter read write timeout " << timeout);
  unsigned char* p = buf;
  size_t read = 0;
  while ( len > read)
  {
    DEBUG( label << ":" << m_desc << " len " << len
           << " read " << read <<" p " << p );
    if  ( timeout != 0 )
    {
      // wait until we can read...
      if (timeout < 0 )
        // no timeout set, we sleep until data is available,
        // but don't return unless we got all data
        poll( mode, 200 );
      else if ( !poll( mode, timeout ) )
      {
        DEBUG( label << ": " << m_desc << " timeout" );
        // timeout occured, return
        break;
      }
    }
    ssize_t nResult=0;
    if ( mode == P_READ )
    {
      boost::mutex::scoped_lock lock( m_readMutex);
      nResult = m_read.read( p, len-read );
    }
    else
    {
      boost::mutex::scoped_lock lock( m_writeMutex);
      nResult = m_write.write( p, len-read );
    }
    if ( nResult < 0 )
    {
      DEBUG( label << " nResult< 0 buffer full/empty?");
      continue;
    }
    if ( nResult == 0 )
    {
      // connection lost
      DEBUG( label << ":" << m_desc << " nrecv return 0 " );
      throw SocketClosedException( label );
    }
    read+=nResult;
    p+=nResult;
  }
  DEBUG( label << ":" << m_desc << " leave read write return:" << read );
  return (size_t) read;
}

//----------------------------------------------------------------------
