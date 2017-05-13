// $Id: CALPacketHeader.h,v 1.15.2.2 2011/12/10 22:47:33 avalassi Exp $
#ifndef CORALSERVERBASE_CALPACKETHEADER_H
#define CORALSERVERBASE_CALPACKETHEADER_H 1

// Include files
#include <string>
#include "CoralServerBase/CALOpcode.h"
#ifdef _WIN32
#include "CoralServerBase/wintypes.h" // For uint32_t on Windows
#else
#include <stdint.h> // For uint32_t on gcc46 (bug #89762)
#endif

namespace coral
{

  /// The CAL packet header size.
  static const uint32_t CALPACKET_HEADER_SIZE = 6;

  /** @class CALPacketHeader
   *
   *  A representation of the CORAL application layer (CAL) packet header.
   *
   *  @author Andrea Valassi, Alexander Kalkhof, Andy Salnikov
   *  @date   2009-01-21
   */

  class CALPacketHeader
  {

  public:

    /// Constructor for send (encode).
    /// Throws an exception if the system is not little-endian.
    /// Cacheable size can be > 0 also for non-cacheable packets.
    /// Cacheable size can be = 0 also for cacheable packets.
    CALPacketHeader( CALOpcode calOpcode,
                     bool calFromProxy,
                     bool calCacheable,
                     uint32_t calCacheableSize );

    /// Constructor for recv.
    /// Throws an exception if the system is not little-endian.
    /// Throws an exception if the input string is too short.
    /// Throws an exception if the reserved flags are not equal to 0.
    CALPacketHeader( const unsigned char* pData, size_t nData );

    /// Destructor (non-virtual).
    ~CALPacketHeader();

    /// Return the header raw data buffer.
    const unsigned char* data() const
    {
      return m_data;
    }

    /// Get the CAL opcode.
    CALOpcode opcode() const;

    /// Get the fromProxy flag.
    bool fromProxy() const;

    /// Get the cacheable flag.
    bool cacheable() const;

    /// Get the size of the cacheable part of the CAL payload.
    uint32_t cacheableSize() const;

    /// Set the fromProxy flag.
    void setFromProxy( bool calFromProxy );

  private:

    /// The standard constructor is private.
    CALPacketHeader();

  private:

    /// The header byte data.
    unsigned char m_data[ CALPACKET_HEADER_SIZE ];

  };

}
#endif // CORALSERVERBASE_CALPACKETHEADER_H
