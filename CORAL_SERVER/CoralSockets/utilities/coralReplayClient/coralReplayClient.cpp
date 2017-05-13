// Port to boost 1.48, preserving backward compatibility to 1.44 (bug #89663)
#define BOOST_FILESYSTEM_VERSION 3

// Include files
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include "CoralBase/boost_filesystem_headers.h"
#include "CoralServerBase/ByteBuffer.h"
#include "CoralServerBase/CTLPacketHeader.h"

// Local include files
#include "../../src/PacketSocket.h"
#include "../../src/TcpSocket.h"
#include "../../src/ThreadManager.h"

#define DEBUG( out ) do { std::cout << out << std::endl; } while (0)
#ifndef DEBUG
#define DEBUG( out )
#endif

namespace coral
{
  namespace CoralSockets
  {

    class ReplayThread  : public Runnable {

    private:
      // copy constructor and assignment operator are private
      ReplayThread( const ReplayThread& rhs );
      ReplayThread& operator=( const ReplayThread& );

    public:

      virtual ~ReplayThread() {
      }

      /// one ReplayThread instance per socket
      ///
      ///
      ReplayThread( const std::string& replayPath,
                    PacketSocketPtr cSocket )
        : Runnable()
        , m_replayPath( replayPath )
        , m_cSocket( cSocket )
        , m_isActive( true )
      {}

      const std::string desc() const
      {
        return "ReplayThread";
      }

      /// main loop of thread
      void operator()();

      void endThread()
      {
        m_isActive=false;
      }

      bool isActive()
      {
        return m_isActive;
      }

      static int getWrongChecksumCount()
      {
        return m_wrongChecksums;
      }

      static int getWrongPacketCount()
      {
        return m_wrongPackets;
      };

    private:

      /// Standard constructor is private
      ReplayThread();

      const std::string m_replayPath;

      PacketSocketPtr m_cSocket;

      bool m_isActive;

      static int m_wrongPackets;

      static int m_wrongChecksums;

      PacketPtr readPacket( const std::string& packetPath );


    };

    int ReplayThread::m_wrongPackets   =0;
    int ReplayThread::m_wrongChecksums =0;

    PacketPtr ReplayThread::readPacket( const std::string& packetPath )
    {

      int fd = ::open( packetPath.c_str(), O_RDONLY );
      if (fd == -1) {
        std::cerr << "error opening file '" << packetPath << "' for reading." << std::endl;
        return PacketPtr( (PacketSLAC *)0 );
      };

      unsigned char header[ CTLPACKET_HEADER_SIZE ];
      int ret = read( fd,  (unsigned char*)header, CTLPACKET_HEADER_SIZE );
      if ( ret == -1 ) {
        std::cerr << "error reading header from file '" << packetPath << "'" << std::endl;
        return PacketPtr( (PacketSLAC *)0 );
      };

      if ( ret != (size_t)CTLPACKET_HEADER_SIZE ) {
        std::cerr << "could not read all bytes of the header from '" << packetPath << "'" << std::endl;
        return PacketPtr( (PacketSLAC *)0 );
      };

      // this is a bit stupid, but I'll leave it for now
      // we need a CTLHeader to know the size
      CTLPacketHeader ctlHeader( (unsigned char*)header, CTLPACKET_HEADER_SIZE);

      DEBUG( "read header payload size : " << ctlHeader.packetSize() << std:: endl
             << "requestID : " << ctlHeader.requestID() );

      // read the payload
      std::auto_ptr<ByteBuffer> payload( new ByteBuffer(
                                                        ctlHeader.packetSize()-CTLPACKET_HEADER_SIZE ) );

      size_t read = 0;
      while (read < payload->freeSize() )
      {
        size_t ret2 = ::read(fd, payload->data()+read, payload->freeSize()-read);
        if (ret2 == (size_t)-1 ) {
          std::cerr<<"could not read payload from file, aborting."<< std::endl;
          return PacketPtr( (PacketSLAC*) 0);
        };
        read +=ret2;
      }
      ::close(fd);
      payload->setUsedSize( payload->freeSize() );
      DEBUG("Read " << read << " bytes of payload. Payloadsize: " << payload->usedSize() );
      // construct PacketPtr
      PacketPtr retPtr( new PacketSLAC( ctlHeader, payload ) );

      return retPtr;
    }


    void ReplayThread::operator()()
    {
      boost::filesystem::path replayPath( m_replayPath );
      if ( !boost::filesystem::exists( replayPath ) ) {
        std::cerr << "Error! Path '" << m_replayPath << "' doesn't exist. Aborting." << std::endl;
        return;
      }
      std::vector<std::string> files;
      boost::filesystem::directory_iterator itr( replayPath );
      boost::filesystem::directory_iterator end_itr;
      std::cout << "Read all files in " << m_replayPath << std::endl;
      while ( itr != end_itr ) {
        std::cout <<"Found : "<< itr->path().filename().string() << std::endl;
        std::string nameEnd = itr->path().filename().string();
        nameEnd = nameEnd.substr( nameEnd.length() - 5 );
        std::cout << nameEnd << std::endl;
        if ( nameEnd != "S.bin" && nameEnd != "R.bin" )
          std::cerr << "Warning! File '" << itr->path().filename().string() << "' is not a packet file: skip it." << std::endl;
        else
          files.push_back( itr->path().filename().string() );
        ++itr;
      }
      std::sort( files.begin(), files.end() );

      for ( std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); file ++ ) {
        if ( !isActive() ) break;
        std::cout << "Handle " << *file << std::endl;
        std::string nameEnd = *file;
        nameEnd = nameEnd.substr( nameEnd.length() - 5 );
        std::cout << "File type: " << nameEnd << std::endl;
        if ( nameEnd != "S.bin" && nameEnd != "R.bin" ) {
          std::cerr << "Error! File '" << itr->path().filename().string() << "' is not a packet file. Aborting." << std::endl;
          return;
        }

        PacketPtr curPkt = readPacket( m_replayPath + "/" + *file );

        if ( nameEnd == "S.bin" ) {
          std::cout << "Waiting to receive packet with requestID: " << curPkt->getHeader().requestID()
                    << std::endl;
          PacketPtr tmp = m_cSocket->receivePacket();
          if ( tmp->getHeader().requestID() == curPkt->getHeader().requestID() ) {
            std::cout << "got the right packet :-)";
            if ( tmp->getHeader().payloadChecksum() == curPkt->getHeader().payloadChecksum() ) {
              std::cout << " and it even has the right checksum :-) " << std::endl;
            } else {
              std::cout << " but the checksum is wrong ++++++++++++++++++++++++++++++++++"
                        << std::endl;
              m_wrongChecksums++;
            }
          } else {
            std::cout <<"+++++++++++++++++++++++ received the wrong id : "
                      << tmp->getHeader().requestID() << std::endl;
            m_wrongPackets++;
          }
        } else {
          std::cout << "Sending packet." << std::endl;
          m_cSocket->sendPacket( *curPkt );
        };
      };
    }

    static bool sortTime(const std::pair<time_t, std::string>& left,
                         const std::pair<time_t, std::string>& right)
    {
      return left.first < right.first;
    }

  }
}

/** @file replayClient.cpp
 *
 *  @author Andrea Valassi and Martin Wache
 *  @date   2007-12-26
 */

using namespace coral::CoralSockets;

int main( int argc, char** argv )
{

  try
  {

    // Get the server parameters
    std::string host;
    std::string replayDir;
    int port;
    bool allConnections = false;
    if ( argc == 4 )
    {
      host = argv[1];
      port = atoi( argv[2] );
      replayDir =  argv[3] ;
    }
    else if ( argc == 5 )
    {
      host = argv[1];
      port = atoi( argv[2] );
      allConnections = true;
      replayDir =  argv[4] ;
    }
    else
    {
      std::cout << "Usage:   " << argv[0] << " [host port replayDir]" << std::endl;
      std::cout << "Usage:   " << argv[0] << " [host port -a replayDir]" << std::endl;
      std::cout << std::endl;
      std::cout << " -a causes the replay client to replay all connections in the directory"
                << std::endl;
      return 1;
    }

    // Create a client
    std::cout << "Entering main" << std::endl;
    std::cout << "Create a client connected to the server on "
              << host << ":" << port << std::endl;
    ThreadManager thrManager;

    if ( allConnections)
    {
      // find all directories with connections and sort them by the time
      boost::filesystem::path replayPath( replayDir );
      if ( !boost::filesystem::exists( replayDir ) ) {
        std::cerr << "Error! Path '" << replayDir << "' doesn't exist. Aborting." << std::endl;
        exit(-1);
      }
      boost::filesystem::directory_iterator itr( replayDir );
      boost::filesystem::directory_iterator end_itr;

      std::vector< std::pair<time_t, std::string> > connectionDirectories;

      while ( itr != end_itr ) {
        std::cout <<"Found : "<< itr->path().filename().string() << std::endl;
        std::string fullPath( replayDir + "/" + itr->path().filename().string() );
        struct stat statbuf;

        if ( stat( fullPath.c_str(), &statbuf ) != 0 ) {
          std::cerr << "could not stat directory '" << fullPath << "' aborting." << std::endl;
          exit( -1 );
        };

        connectionDirectories.push_back( std::make_pair( statbuf.st_mtime, fullPath ) );
        ++itr;
      };
      sort(connectionDirectories.begin(), connectionDirectories.end(),&sortTime);


      std::cout << "Will replay the following connections: " <<std::endl;
      time_t start = connectionDirectories.begin()->first;

      for ( std::vector< std::pair<time_t, std::string> >::const_iterator
              itr2 = connectionDirectories.begin();
            itr2 != connectionDirectories.end(); ++itr2 )
      {
        std::cout << itr2->second << " after "
                  << itr2->first - start << " seconds." << std::endl;
      };
      std::vector< std::pair<time_t, std::string> >::const_iterator it = connectionDirectories.begin();
      while ( it != connectionDirectories.end() ) {
        std::cout << "sleeping " << it->first -start << " seconds." << std::endl;
        sleep( it->first - start );

        std::cout << "starting replay of '" << it->second << "'" << std::endl;
        TcpSocketPtr socket( TcpSocket::connectTo(host,port) );
        thrManager.addThread( new ReplayThread( it->second, PacketSocketPtr(
                                                                            new PacketSocket( socket) ) ) );
        ++it;
      }
    }
    else
    {
      // replay only a single connection

      TcpSocketPtr socket( TcpSocket::connectTo(host,port) );
      thrManager.addThread( new ReplayThread( replayDir, PacketSocketPtr( new PacketSocket( socket) ) ) );
    }
    thrManager.joinAll();
    std::cout << "Exiting main" << std::endl;
  }

  catch( std::exception& e )
  {
    std::cout << "ERROR! Standard C++ exception: '" << e.what() << "'" << std::endl;
    return 1;
  }

  catch( ... )
  {
    std::cout << "ERROR! Unknown exception caught" << std::endl;
    return 1;
  }

  std::cerr << "got " << ReplayThread::getWrongPacketCount() << " wrong packet ids " << std::endl
            << "and " << ReplayThread::getWrongChecksumCount() << " wrong Checksums " << std::endl;

  //printTimers();
  // Successful program termination
  return 0;

}
