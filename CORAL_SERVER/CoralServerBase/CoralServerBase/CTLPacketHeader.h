// $Id: CTLPacketHeader.h,v 1.21.2.2 2011/12/10 22:47:39 avalassi Exp $
#ifndef CORALSERVERBASE_CTLPACKETHEADER_H
#define CORALSERVERBASE_CTLPACKETHEADER_H 1

// Include files
#include <string>
#include "CoralServerBase/CTLStatus.h"
#ifdef _WIN32
#include "CoralServerBase/wintypes.h" // For uint32_t on Windows
#else
#include <stdint.h> // For uint32_t on gcc46 (bug #89762)
#endif

namespace coral
{

  /// The CTL packet header size.
  static const uint32_t CTLPACKET_HEADER_SIZE = 23;

  /// The CTL packet magic word { 0xFE = 254, 0xED = 237 }.
  static const unsigned char CTLPACKET_MAGIC_WORD[2] = { 0xFE, 0xED };

  /// The current CORAL protocol version.
  /// This refers to both the CTL and CAL protocol.
  static const uint32_t CTLPACKET_CURRENT_VERSION = 0;

  /// The CTL packet header size.
  //static const uint32_t CTLPACKET_MAX_SIZE = 16777215; // 16MB-1 (0x00FFFFFF)
  //static const uint32_t CTLPACKET_MAX_SIZE = 16383; // 16kB-1 (0x00003FFF)
  static const uint32_t CTLPACKET_MAX_SIZE = 65535; // 64kB-1 (0x0000FFFF)

  /** @class CTLPacketHeader
   *
   *  A representation of the CORAL transport layer (CTL) packet header.
   *
   *  Some of the metadata in the header depends on the CTL payload.
   *  First, the CTL packet size should be set equal to the sum
   *  of the CTL header size and of the CTL payload size.
   *  [NB: the payload size must be at least 6 bytes (CAL header)!]
   *  Second, the CTL payload checksum should be computed using the
   *  algorithm implemented in the computeChecksum() method.
   *  [NB: the first byte in the payload is skipped (proxy flags)!]
   *
   *  The maximum allowed size of a CTL packet (header and payload)
   *  is CTLPACKET_MAX_SIZE. This is internally checked by this class.
   *
   *  @author Andrea Valassi, Martin Wache, Andy Salnikov
   *  @date   2009-01-20
   */

  class CTLPacketHeader
  {

  public:

    /// Constructor for send (encode).
    /// System-provided: magic word, protocol version.
    /// Throws an exception if the system is not little-endian.
    /// Throws an exception if packet size is too short or too long.
    CTLPacketHeader( CTLStatus ctlStatus,
                     uint32_t ctlPacketSize,
                     uint32_t ctlRequestID,
                     uint32_t ctlClientID,
                     uint32_t ctlSegmentNumber,
                     bool ctlMoreSegments,
                     uint32_t ctlPayloadChecksum );

    /// Constructor for recv.
    /// Throws an exception if system is not little-endian.
    /// Throws an exception if input string is too short.
    /// Throws WrongMagicWord input string does not start by the magic word.
    /// Throws an exception if encoded packet size is too short or too long.
    CTLPacketHeader( const unsigned char* pData, size_t nData );

    /// Destructor (non-virtual).
    ~CTLPacketHeader();

    /// Return the header raw data buffer.
    const unsigned char* data() const
    {
      return m_data;
    }

    /// Get the protocol version (for this packet).
    uint32_t version() const;

    /// Get the reply status.
    CTLStatus status() const;

    /// Get the packet size.
    uint32_t packetSize() const;

    /// Get the requestID.
    uint32_t requestID() const;

    /// Get the clientID.
    uint32_t clientID() const;

    /// Get the segment number.
    uint32_t segmentNumber() const;

    /// Get the "more segments" flag.
    bool moreSegments() const;

    /// Get the payload checksum.
    uint32_t payloadChecksum() const;

    /// Compute the payload checksum.
    /// Throws if nCtlPayload < 6 (CAL header size).
    /// [NB: the 1st byte is skipped in the computation]
    static uint32_t computeChecksum( const unsigned char* pCtlPayload,
                                     size_t nCtlPayload );

    /// Set the clientID.
    void setClientID( uint32_t clientID );

    /// Set the payload checksum.
    void setPayloadChecksum( uint32_t payloadChecksum );

  private:

    /// The standard constructor is private.
    CTLPacketHeader();

  private:

    /// The header byte data.
    unsigned char m_data[ CTLPACKET_HEADER_SIZE ];

  };

}
#endif // CORALSERVERBASE_CTLPACKETHEADER_H
