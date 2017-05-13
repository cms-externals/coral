// $Id: CTLPacketHeader.cpp,v 1.33.2.2 2011/03/08 15:01:28 avalassi Exp $

// Include files
#include <cstring> // For memcpy
#include <cstdlib> // For getenv
#include <iostream>
#include <sstream>
#include "crc32.h"
#include "CoralMonitor/StopTimer.h"
#include "CoralMonitor/StatsTypeTimer.h"
//#include "CoralServerBase/hexstring.h"
#include "CoralServerBase/InternalErrorException.h"
#include "CoralServerBase/CALPacketHeader.h"
#include "CoralServerBase/CTLPacketHeader.h"
#include "CoralServerBase/CTLMagicWordException.h"


// Namespace
using namespace coral;

static StatsTypeTimer myStatsTypeTimer("CBase_ComputeChecksum", "compute checksum");

//-----------------------------------------------------------------------------

CTLPacketHeader::~CTLPacketHeader()
{
}

//-----------------------------------------------------------------------------

CTLPacketHeader::CTLPacketHeader( CTLStatus ctlStatus,
                                  uint32_t ctlPacketSize,
                                  uint32_t ctlRequestID,
                                  uint32_t ctlClientID,
                                  uint32_t ctlSegmentNumber,
                                  bool ctlMoreSegments,
                                  uint32_t ctlPayloadChecksum )
{
#if ( __BYTE_ORDER == __LITTLE_ENDIAN )
  // OK System uses little-endian byte ordering
#elif ( __BYTE_ORDER == __BIG_ENDIAN )
  throw InternalErrorException( "System uses big-endian byte ordering",
                                "CTLPacketHeader::CTLPacketHeader #1",
                                "coral::CoralServerBase" );
#else
  throw InternalErrorException( "System uses neither little-endian nor big-endian byte ordering",
                                "CTLPacketHeader::CTLPacketHeader #1",
                                "coral::CoralServerBase" );
#endif

  // Encode magic word
  m_data[0] = CTLPACKET_MAGIC_WORD[0];
  m_data[1] = CTLPACKET_MAGIC_WORD[1];

  // Encode version and reply status
  //std::cout << "0x3F = " << 0x3F << std::endl; // 63
  //std::cout << "0x3Fu = " << 0x3Fu << std::endl; // 63
  unsigned char c2 = CTLPACKET_CURRENT_VERSION;
  if ( c2 > 0x3F )
    throw InternalErrorException( "PANIC! Current version > 0x3F",
                                  "CTLPacketHeader::CTLPacketHeader #1",
                                  "coral::CoralServerBase" );
  switch ( ctlStatus )
  {
  case CTLOK:             c2 = c2 | 0x00; break;
  case CTLWrongMagicWord: c2 = c2 | 0x40; break;
  case CTLWrongVersion:   c2 = c2 | 0x80; break;
  case CTLWrongChecksum:  c2 = c2 | 0xC0; break;
  }
  m_data[2] = c2;

  // Check and encode packet size
  if ( ctlPacketSize < CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE )
    throw Exception( "CTL packet is shorter than CTL+CAL headers",
                     "CTLPacketHeader::CTLPacketHeader #1",
                     "coral::CoralServerBase" );
  if ( ctlPacketSize > CTLPACKET_MAX_SIZE )
    throw Exception( "CTL packet is longer than CTLPACKET_MAX_SIZE",
                     "CTLPacketHeader::CTLPacketHeader #1",
                     "coral::CoralServerBase" );
  ::memcpy( m_data+3, &ctlPacketSize, 4 );

  // Encode requestID
  ::memcpy( m_data+7, &ctlRequestID, 4 );

  // Encode clientID
  ::memcpy( m_data+11, &ctlClientID, 4 );

  // Encode segment number and "more segments" flag
  //std::cout << "0x7FFFFFFF = " << 0x7FFFFFFF << std::endl; // 2147483647
  //std::cout << "(ui)0x7FFFFFFF = " << (uint32_t)0x7FFFFFFF << std::endl; // 2147483647
  //std::cout << "0xFFFFFFFF = " << 0xFFFFFFFF << std::endl; // 4294967295
  //std::cout << "(ui)0xFFFFFFFF = " << (uint32_t)0xFFFFFFFF << std::endl; // 4294967295
  if ( ctlSegmentNumber > (uint32_t)0x7FFFFFFF )
  {
    std::stringstream msg;
    msg << "Segment number " << ctlSegmentNumber << " is > 0x7FFFFFFF";
    throw Exception( msg.str(),
                     "CTLPacketHeader::CTLPacketHeader #1",
                     "coral::CoralServerBase" );
  }
  uint32_t i15 = ctlSegmentNumber;
  if ( ctlMoreSegments ) i15 = i15 | 0x80000000;
  ::memcpy( m_data+15, &i15, 4 );

  // Encode payload checksum
  ::memcpy( m_data+19, &ctlPayloadChecksum, 4 );

}

//-----------------------------------------------------------------------------

CTLPacketHeader::CTLPacketHeader( const unsigned char* pData, size_t nData )
{
#if ( __BYTE_ORDER == __LITTLE_ENDIAN )
  // OK System uses little-endian byte ordering
#elif ( __BYTE_ORDER == __BIG_ENDIAN )
  throw InternalErrorException( "System uses big-endian byte ordering",
                                "CTLPacketHeader::CTLPacketHeader #2",
                                "coral::CoralServerBase" );
#else
  throw InternalErrorException( "System uses neither little-endian nor big-endian byte ordering",
                                "CTLPacketHeader::CTLPacketHeader #2",
                                "coral::CoralServerBase" );
#endif

  // Check total header size
  if ( nData < CTLPACKET_HEADER_SIZE )
    throw Exception( "Too few bytes to build a CTLPacketHeader",
                     "CTLPacketHeader::CTLPacketHeader #2",
                     "coral::CoralServerBase" );

  // Check magic word
  unsigned char c0 = *(pData+0);
  unsigned char c1 = *(pData+1);
  //std::cout << "Check magic word: '" << hexstring(pData,nData)
  //          << "', '" << hexstring(CTLPACKET_MAGIC_WORD,2)
  //          << "', " << pos << std::endl;
  if ( c0 != CTLPACKET_MAGIC_WORD[0] || c1 != CTLPACKET_MAGIC_WORD[1] )
    throw CTLMagicWordException( "CTLPacketHeader::CTLPacketHeader #2" );

  // Check packet size
  uint32_t ctlPacketSize;
  ::memcpy( &ctlPacketSize, pData+3, 4 );
  if ( ctlPacketSize < CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE )
    throw Exception( "CTL packet is shorter than CTL+CAL headers",
                     "CTLPacketHeader::CTLPacketHeader #2",
                     "coral::CoralServerBase" );
  if ( ctlPacketSize > CTLPACKET_MAX_SIZE )
    throw Exception( "CTL packet is longer than CTLPACKET_MAX_SIZE",
                     "CTLPacketHeader::CTLPacketHeader #2",
                     "coral::CoralServerBase" );

  // Copy the header raw buffer data
  ::memcpy( m_data, pData, CTLPACKET_HEADER_SIZE );
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::version() const
{
  unsigned char c2 = *(m_data+2);
  return c2 & 0x3F;
}

//-----------------------------------------------------------------------------

CTLStatus CTLPacketHeader::status() const
{
  unsigned char c2 = *(m_data+2);
  switch ( c2 & 0xC0 )
  {
  case 0x00: return CTLOK; break;
  case 0x40: return CTLWrongMagicWord; break;
  case 0x80: return CTLWrongVersion; break;
  case 0xC0: return CTLWrongChecksum; break;
  default:
    throw InternalErrorException( "PANIC! Could not decode CTLStatus",
                                  "CTLPacketHeader::status",
                                  "coral::CoralServerBase" );
  }
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::packetSize() const
{
  uint32_t i3;
  ::memcpy( &i3, m_data+3, 4 );
  if ( i3 < CTLPACKET_HEADER_SIZE + CALPACKET_HEADER_SIZE )
    throw Exception( "CTL packet is shorter than CTL+CAL headers",
                     "CTLPacketHeader::packetSize()",
                     "coral::CoralServerBase" );
  if ( i3 > CTLPACKET_MAX_SIZE )
    throw Exception( "CTL packet is longer than CTLPACKET_MAX_SIZE",
                     "CTLPacketHeader::packetSize()",
                     "coral::CoralServerBase" );
  return i3;
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::requestID() const
{
  uint32_t i7;
  ::memcpy( &i7, m_data+7, 4 );
  return i7;
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::clientID() const
{
  uint32_t i11;
  ::memcpy( &i11, m_data+11, 4 );
  return i11;
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::segmentNumber() const
{
  uint32_t i15;
  ::memcpy( &i15, m_data+15, 4 );
  return i15 & 0x7FFFFFFF;
}

//-----------------------------------------------------------------------------

bool CTLPacketHeader::moreSegments() const
{
  uint32_t i15;
  ::memcpy( &i15, m_data+15, 4 );
  return ( i15 > 0x7FFFFFFF ? true : false );
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::payloadChecksum() const
{
  uint32_t i19;
  ::memcpy( &i19, m_data+19, 4 );
  return i19;
}

//-----------------------------------------------------------------------------

uint32_t CTLPacketHeader::computeChecksum( const unsigned char* pData, size_t nData )
{
  ScopedTimerStats timer(myStatsTypeTimer);

  if ( nData < CALPACKET_HEADER_SIZE )
    throw Exception( "CTL payload is shorter than a CAL header",
                     "CTLPacketHeader::computeChecksum",
                     "coral::CoralServerBase" );

  static bool checksumDisabled = ::getenv("CORALSERVER_NOCRC") != 0;
  if (checksumDisabled)
    return 0 ;

  uint32_t checksum = 0;

  return Crc32_ComputeBuf( checksum, pData+1, nData-1);
}

//-----------------------------------------------------------------------------

void CTLPacketHeader::setClientID( uint32_t ctlClientID )
{
  ::memcpy( m_data+11, &ctlClientID, 4 );
}

//-----------------------------------------------------------------------------

void CTLPacketHeader::setPayloadChecksum( uint32_t ctlPayloadChecksum )
{
  ::memcpy( m_data+19, &ctlPayloadChecksum, 4 );
}

//-----------------------------------------------------------------------------
