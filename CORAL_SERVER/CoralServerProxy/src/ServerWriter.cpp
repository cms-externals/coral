//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id: ServerWriter.cpp,v 1.2.2.1 2010/05/26 08:12:41 avalassi Exp $
//
// Description:
//	Class ServerWriter...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "ServerWriter.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "MsgLogger.h"
#include "Packet.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace coral {
namespace CoralServerProxy {

//----------------
// Constructors --
//----------------
ServerWriter::ServerWriter ( PacketQueue& queue, NetSocket& socket )
  : m_queue ( queue )
  , m_socket ( socket )
{
}

//--------------
// Destructor --
//--------------
ServerWriter::~ServerWriter ()
{
}

// this is the "run" method used by the Boost.thread
void
ServerWriter::operator() ()
{
  while ( true ) {

    // receive next packet from a queue
    PacketPtr p = m_queue.pop() ;
    PXY_DEBUG ( "ServerWriter " << m_socket << ": got packet from queue " << *p );

    if ( p->type() == Packet::Control ) {
      PXY_DEBUG ( "ServerWriter " << m_socket << ": got control packet" );
    } else {
      // send it down to the wire
      int s = p->write ( m_socket ) ;
      if ( s <= 0 ) {
        PXY_ERR ( "ServerWriter " << m_socket << ": failed to send data to the server" );
      }
    }

  }
}

} // namespace CoralServerProxy
} // namespace coral
