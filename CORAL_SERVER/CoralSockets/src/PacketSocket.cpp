// $Id: PacketSocket.cpp,v 1.21.2.3 2011/09/16 15:20:06 avalassi Exp $

// Include files
#include <cerrno>
#include <cstring> // For strerror
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include "CoralMonitor/StatsTypeBandwidth.h"
#include "CoralSockets/GenericSocketException.h"

// Local include files
#include "PacketSocket.h"
#include "StatsTypePackets.h"

// Logger
#define LOGGER_NAME "CoralSockets::PacketSocket"
#include "logger.h"

// Debug
#undef DEBUG
#define DEBUG( out )

// Namespace
using namespace coral::CoralSockets;

//---------------------------------------------------------------------------

//create an object for statistics
static StatsTypePackets mySTPacketsOut("CSocket_POut","packets out");
static StatsTypePackets mySTPacketsIn("CSocket_PIn","packets in");
static coral::StatsTypeBandwidth mySTBandwidthOut("CSocket_BOut","bandwidth out");
static coral::StatsTypeBandwidth mySTBandwidthIn("CSocket_BIn","bandwidth in");

//---------------------------------------------------------------------------

static std::string dumpPath("/tmp");

static bool dumpEnabled( bool disable = false )
{
  static bool gotPath = false;
  static bool isDumpEnabled = getenv("CORALSERVER_DUMPPACKET_PATH") != 0;
  if ( disable ) isDumpEnabled = false;
  if ( !isDumpEnabled ) return false;
  if ( isDumpEnabled && !gotPath )
  {
    dumpPath = std::string( getenv("CORALSERVER_DUMPPACKET_PATH") );
    gotPath = true;
  }
  return true;
}

//---------------------------------------------------------------------------

static inline uint64_t getTimeUS()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64_t)tv.tv_sec*1000000+(uint64_t)tv.tv_usec;
}

//---------------------------------------------------------------------------

PacketSocket::PacketSocket( ISocketPtr socket)
  : m_socket( socket )
  , m_createTime( getTimeUS() )
  , m_recPackets( 0 )
  , m_sendPackets( 0 )
{
}

//---------------------------------------------------------------------------

PacketSocket::~PacketSocket()
{
  INFO("PacketSocket '"<<m_socket->desc()<<"' rec : " << m_recPackets
       << " packets send : " << m_sendPackets << " packets");
  if ( getenv("CORALSERVER_STATS") )
    std::cout << "PacketSocket '"<<m_socket->desc()<<"' rec : " << m_recPackets
              << " packets send : " << m_sendPackets << " packets"<< std::endl;
}

//---------------------------------------------------------------------------

void PacketSocket::sendPacket( const PacketSLAC& Message)
{
  // - cork, uncork?
  // - avoid deep copies
  const CTLPacketHeader &ctlHeader( Message.getHeader() );
  const ByteBuffer &payload( Message.getPayload() );
  DEBUG("dumping header ver:" << ctlHeader.version()
        << " status:"  << ctlHeader.status()
        << " size:" << ctlHeader.packetSize()
        << " requestID:" << ctlHeader.requestID()
        << " clientID:" << ctlHeader.clientID()
        << " segmentNumber:" << ctlHeader.segmentNumber()
        << " moreSegments: " << ctlHeader.moreSegments()
        << std::endl
        );
  if ( dumpEnabled() ) dumpPacket( dumpPath, true, Message);
  m_socket->cork();
  m_socket->writeAll( ctlHeader.data(), CTLPACKET_HEADER_SIZE );
  m_socket->writeAll( payload.data(), payload.usedSize() );
  m_socket->uncork();
  m_sendPackets++;
  mySTPacketsOut.add( CTLPACKET_HEADER_SIZE, payload.usedSize() );
  mySTBandwidthOut.add( CTLPACKET_HEADER_SIZE + payload.usedSize() );
}

//---------------------------------------------------------------------------

PacketPtr PacketSocket::receivePacket( )
{
  try
  {
    // receive the header
    unsigned char header[ CTLPACKET_HEADER_SIZE ];
    DEBUG("reading header" << std::endl);
    m_socket->readAll( (unsigned char*)header, CTLPACKET_HEADER_SIZE );
    // this is a bit stupid, but I'll leave it for now
    // we need a CTLHeader to know the size
    CTLPacketHeader ctlHeader( (unsigned char*)header, CTLPACKET_HEADER_SIZE);
    DEBUG("dumping header ver:" << ctlHeader.version()
          << " status:"  << ctlHeader.status()
          << " size:" << ctlHeader.packetSize()
          << " requestID:" << ctlHeader.requestID()
          << " clientID:" << ctlHeader.clientID()
          << " segmentNumber:" << ctlHeader.segmentNumber()
          << " moreSegments: " << ctlHeader.moreSegments()
          << std::endl);
    // first check: version
    if ( ctlHeader.version() != CTLPACKET_CURRENT_VERSION )
      throw ErrorInHeaderException(ErrorInHeaderException::ErrWrongVersion,
                                   "received packet with wrong version number!",
                                   "PacketSocket::receivePacket()");

    // second check: status
    if ( ctlHeader.status() != CTLOK )
    {
      if ( ctlHeader.status() == CTLWrongMagicWord )
        throw PacketStatusNotOkException("Received a packet with status WrongMagicWord",
                                         "PacketSocket::receivePacket()");
      else if ( ctlHeader.status() == CTLWrongVersion )
        throw PacketStatusNotOkException("Received a packet with status WrongVersion",
                                         "PacketSocket::receivePacket()");
      else if ( ctlHeader.status() == CTLWrongChecksum )
        throw PacketStatusNotOkException("Received a packet with status WrongChecksum",
                                         "PacketSocket::receivePacket()");
    }
    // ctlHeader.packetSize() throws if the size is smaller than < CLT+CAL
    // or the packet is larger than CTLPACKET_MAX_SIZE
    // so we only have to make sure ctlHeader is called and catch the exceptions
    if ( ctlHeader.packetSize() == 0 )
      throw GenericSocketException("PANIC! packet size ==0?!",
                                   "PacketSocket::receivePacket()");
    // receive the payload
    std::unique_ptr<ByteBuffer> payload( new ByteBuffer(
                                                      ctlHeader.packetSize()-CTLPACKET_HEADER_SIZE ) );
    DEBUG("reading payload" << std::endl);
    m_socket->readAll( payload->data(), payload->freeSize() );
    payload->setUsedSize( payload->freeSize() );
    static bool checksumDisabled = getenv("CORALSERVER_NOCRC") != 0;
    if ( !checksumDisabled && ctlHeader.payloadChecksum() != 0
         && ctlHeader.payloadChecksum() !=
         CTLPacketHeader::computeChecksum( payload->data(),
                                           payload->usedSize() ) )
      throw ErrorInHeaderException( ErrorInHeaderException::ErrWrongChecksum,
                                    "payload checksum in received header is wrong!",
                                    "PacketSocket::receivePacket()");
    mySTPacketsIn.add( CTLPACKET_HEADER_SIZE, payload->usedSize() );
    mySTBandwidthIn.add( CTLPACKET_HEADER_SIZE + payload->usedSize() );
    // construct PacketPtr
    PacketPtr ret( new PacketSLAC( ctlHeader, payload ) );
    m_recPackets++;
    if ( dumpEnabled() ) dumpPacket( dumpPath, false, ret);
    return ret;
  }
  catch (Exception &e) {
    const std::string expMsgShort="CTL packet is shorter than CTL+CAL headers";
    const std::string expMsgLong="CTL packet is longer than CTLPACKET_MAX_SIZE";
    if ( expMsgShort == std::string( e.what(), expMsgShort.size() ) ) {
      throw ErrorInHeaderException(ErrorInHeaderException::ErrPacketTooSmall,
                                   "packet size in received header is too small!",
                                   "PacketSocket::receivePacket()");
    } else if ( expMsgLong == std::string( e.what(), expMsgLong.size() ) ) {
      throw ErrorInHeaderException(ErrorInHeaderException::ErrPacketTooLarge,
                                   "packet size in received header is too large!",
                                   "PacketSocket::receivePacket()");
    } else throw;
  }
}

//---------------------------------------------------------------------------

bool PacketSocket::isOpen()
{
  return m_socket->isOpen();
}

//---------------------------------------------------------------------------

void PacketSocket::close()
{
  return m_socket->close();
}

//---------------------------------------------------------------------------

int PacketSocket::getFd()
{
  return m_socket->getFd();
}

//---------------------------------------------------------------------------

void PacketSocket::dumpPacket( const std::string& path, bool Send, const PacketPtr& packet )
{
  dumpPacket( path, Send, (*packet) );
}

//---------------------------------------------------------------------------

void PacketSocket::dumpPacket( const std::string& path, bool Send, const PacketSLAC& packet )
{
  std::ostringstream fullPath;
  fullPath << path << "/" << m_socket->desc();
  int error=mkdir( fullPath.str().c_str(), 0777);
  if (error != 0 && errno !=EEXIST )
  {
    ERROR( "could not create packet dump directory '"
           << fullPath <<"' error: " << strerror(errno) << "!" );
    dumpEnabled(false);
    return;
  }
  std::ostringstream filename;
  filename << fullPath.str() << "/";
  filename << std::right << std::setw(12) << std::setfill('0');
  filename << getTimeUS() - m_createTime;
  if (Send) filename << "S.bin";
  else filename << "R.bin";
  int fd = ::open( filename.str().c_str(), O_WRONLY | O_CREAT, 0666 );
  if (fd == -1)
  {
    ERROR("error opening file '" << filename.str() << "' for writing.");
    return;
  }
  size_t written=write(fd, packet.getHeader().data(), CTLPACKET_HEADER_SIZE );
  if (written == (size_t)-1 )
  {
    ERROR("could not write packet header to file, aborting. Error: " << strerror(errno) );
    return;
  }
  if (written != CTLPACKET_HEADER_SIZE )
  {
    ERROR("could not write all bytes of the header, aborting.");
    return;
  }
  written = 0;
  while (written < packet.getPayload().usedSize() )
  {
    size_t ret = ::write(fd, packet.getPayload().data()+written,
                         packet.getPayload().usedSize()-written);
    if (ret == (size_t)-1 )
    {
      ERROR("could not write payload to fiel, aborting.");
      return;
    }
    written +=ret;
  }
  ::close(fd);
}

//---------------------------------------------------------------------------
