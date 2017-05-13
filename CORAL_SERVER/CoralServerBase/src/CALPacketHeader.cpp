// $Id: CALPacketHeader.cpp,v 1.14.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include <cstring> // For memcpy
#include <iostream>
#include <sstream>
#include "CoralBase/Exception.h"
#include "CoralServerBase/CALPacketHeader.h"

// Namespace
using namespace coral;

//-----------------------------------------------------------------------------

CALPacketHeader::~CALPacketHeader()
{
}

//-----------------------------------------------------------------------------

CALPacketHeader::CALPacketHeader( unsigned char calOpcode,
                                  bool calFromProxy,
                                  bool calCacheable,
                                  uint32_t calCacheableSize )
{
#if ( __BYTE_ORDER == __LITTLE_ENDIAN )
  // OK System uses little-endian byte ordering
#elif ( __BYTE_ORDER == __BIG_ENDIAN )
  throw Exception( "System uses big-endian byte ordering",
                   "CALPacketHeader::CALPacketHeader",
                   "coral::CoralServerBase" );
#else
  throw Exception( "System uses neither little-endian nor big-endian byte ordering",
                   "CALPacketHeader::CALPacketHeader",
                   "coral::CoralServerBase" );
#endif

  // Encode fromProxy and cacheable flags
  m_data[0] = 0x00;
  if ( calFromProxy ) m_data[0] |= 0x40;
  if ( calCacheable ) m_data[0] |= 0x80;

  // Encode opcode
  m_data[1] = (unsigned char)calOpcode;

  // Encode cacheable size
  ::memcpy( m_data+2, &calCacheableSize, 4 );

}

//-----------------------------------------------------------------------------

CALPacketHeader::CALPacketHeader( const unsigned char* pData, size_t nData )
{
#if ( __BYTE_ORDER == __LITTLE_ENDIAN )
  // OK System uses little-endian byte ordering
#elif ( __BYTE_ORDER == __BIG_ENDIAN )
  throw Exception( "System uses big-endian byte ordering",
                   "CALPacketHeader::CALPacketHeader",
                   "coral::CoralServerBase" );
#else
  throw Exception( "System uses neither little-endian nor big-endian byte ordering",
                   "CALPacketHeader::CALPacketHeader",
                   "coral::CoralServerBase" );
#endif

  // Check total header size
  if ( nData < CALPACKET_HEADER_SIZE )
    throw Exception( "Too few bytes to build a CALPacketHeader",
                     "CALPacketHeader::CALPacketHeader",
                     "coral::CoralServerBase" );

  // Check reserved flags
  unsigned char c1 = *(pData+0);
  if ( ( c1 & 0x3F ) != 0x00 )
    throw Exception( "Reserved flags are not = 0",
                     "CALPacketHeader::CALPacketHeader",
                     "coral::CoralServerBase" );

  // Copy the header raw buffer data
  ::memcpy( m_data, pData, CALPACKET_HEADER_SIZE );

}

//-----------------------------------------------------------------------------

unsigned char CALPacketHeader::opcode() const
{
  return *(m_data+1);
}

//-----------------------------------------------------------------------------

bool CALPacketHeader::fromProxy() const
{
  unsigned char c1 = *(m_data+0);
  return ( c1 & 0x40 ) == 0x40;
}

//-----------------------------------------------------------------------------

bool CALPacketHeader::cacheable() const
{
  unsigned char c1 = *(m_data+0);
  return ( c1 & 0x80 ) == 0x80;
}

//-----------------------------------------------------------------------------

uint32_t CALPacketHeader::cacheableSize() const
{
  uint32_t res;
  ::memcpy( &res, m_data+2, 4 );
  return res;
}

//-----------------------------------------------------------------------------

void CALPacketHeader::setFromProxy( bool calFromProxy )
{
  if ( calFromProxy ) m_data[0] |= 0x40;
  else m_data[0] &= ~0x40;
}

//-----------------------------------------------------------------------------
