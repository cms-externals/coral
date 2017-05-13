// $Id: CALOpcode.h,v 1.4.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_CALOPCODE_H
#define CORALSERVERBASE_CALOPCODE_H 1

// Include files
#include "CoralServerBase/hexstring.h"
#include "CoralServerBase/CALOpcodeException.h"

namespace coral
{
  /// CAL opcode type definition.
  typedef unsigned char CALOpcode;

  /** @namespace CALOpcodes
   *
   *  Definition of the opcodes of a CORAL application layer request or reply.
   *
   *  The two upper bits determine the opcode type.
   *  Opcodes 00xxxxxx are used for CAL requests.
   *  Opcodes 01xxxxxx are used for CAL replies returning OK.
   *  Opcodes 10xxxxxx are used for CAL replies returning an exception.
   *  Opcodes 11xxxxxx are reserved for non-CAL tests.
   *
   *  The OK reply opcode for a given request 00xxxxxx is 01xxxxxx.
   *  The exception reply opcode for a given request 00xxxxxx is 10xxxxxx.
   *
   *  Only seven request opcodes are publicly available in CoralServerBase.
   *  The remaining request opcodes are private to the CoralStubs package.
   *
   *  @author Andrea Valassi and Alexander Kalkhof
   *  @date   2009-01-23
   */

  namespace CALOpcodes
  {

    // Public opcodes for requests.
    // Add 0x40 for the corresponding opcodes for OK replies.
    // Add 0x80 for the corresponding opcodes for exception replies.
    static const CALOpcode ConnectRO           = 0x01;
    static const CALOpcode ConnectRW           = 0x02;
    static const CALOpcode Disconnect          = 0x03;
    static const CALOpcode StartTransactionRO  = 0x04;
    static const CALOpcode StartTransactionRW  = 0x05;
    static const CALOpcode CommitTransaction   = 0x06;
    static const CALOpcode RollbackTransaction = 0x07;

    /// Is this a reserved opcode for non-CAL tests?
    /// Check if the two upper bits are 0xC0.
    inline bool isReserved( CALOpcode o )
    {
      return ( ( o & 0xC0 ) == 0xC0 );
    }

    /// Is this the opcode of a CAL request?
    /// Check if the two upper bits are 0x00.
    inline bool isCALRequest( CALOpcode o )
    {
      return ( ( o & 0xC0 ) == 0x00 );
    }

    /// Is this the opcode of a CAL reply returning OK?
    /// Check if the two upper bits are 0x40.
    inline bool isCALReplyOK( CALOpcode o )
    {
      return ( ( o & 0xC0 ) == 0x40 );
    }

    /// Is this the opcode of a CAL reply returning OK?
    /// Check if the two upper bits are 0x80.
    inline bool isCALReplyException( CALOpcode o )
    {
      return ( ( o & 0xC0 ) == 0x80 );
    }

    /// Is this the opcode of a CAL reply returning OK?
    /// Check if the two upper bits are 0x40 or 0x80
    inline bool isCALReply( CALOpcode o )
    {
      return ( ( o & 0xC0 ) == 0x40 ) || ( ( o & 0xC0 ) == 0x80 );
    }

    /// Get the request opcode for the given reply opcode.
    /// Throws CALOpcodeException if it is not a reply opcode.
    inline CALOpcode getCALRequest( CALOpcode o )
    {
      if ( !isCALReply( o ) )
      {
        throw CALOpcodeException( "Opcode " + hexstring(&o,1) +
                                  " is not a reply opcode",
                                  "CALOpcodes::getCALRequest" );
      }
      return ( o & 0x3F ); // strip two highest bits
    }

    /// Get the OK reply opcode for the given request opcode.
    /// Throws CALOpcodeException if it is not a request opcode.
    inline CALOpcode getCALReplyOK( CALOpcode o )
    {
      if ( !isCALRequest( o ) )
      {
        throw CALOpcodeException( "Opcode " + hexstring(&o,1) +
                                  " is not a request opcode",
                                  "CALOpcodes::getCALRequest" );
      }
      return ( o | 0x40 );
    }

    /// Get the OK reply opcode for the given request opcode.
    /// Throws CALOpcodeException if it is not a request opcode.
    inline CALOpcode getCALReplyException( CALOpcode o )
    {
      if ( !isCALRequest( o ) )
      {
        throw CALOpcodeException( "Opcode " + hexstring(&o,1) +
                                  " is not a request opcode",
                                  "CALOpcodes::getCALRequest" );
      }
      return ( o | 0x80 );
    }

  }

}
#endif // CORALSERVERBASE_CALOPCODE_H
