// $Id: ISocket.h,v 1.5.2.1 2010/12/20 09:10:11 avalassi Exp $
#ifndef CORALSOCKETS_ISOCKET_H
#define CORALSOCKETS_ISOCKET_H 1

#include "CoralBase/Exception.h"
#include <boost/shared_ptr.hpp>

namespace coral {

  // forward declaration
  class ICertificateData;

  namespace CoralSockets {

    // forward declaration
    class ISocket;

    typedef boost::shared_ptr<ISocket> ISocketPtr;

    class ISocket {
    public:
      enum PMode {
        P_READ,
        P_WRITE,
        P_READ_WRITE
      };

      /// reads len bytes from the socket into *buf
      /// doesn't return until all bytes are read, or some error occurs
      virtual void readAll( unsigned char *buf, size_t len) = 0;

      /// reads at most len bytes from the socket into *buf
      /// returns the number of bytes actually read
      /// returns early if it could not read anything for timeout miliseconds
      /// timeout equal to <0 means try for ever
      virtual size_t read( unsigned char *buf, size_t len,
                           int timeout = 0) = 0;

      /// writes len bytes into the socket into *buf
      /// doesn't return until all bytes are read, or some error occurs
      virtual void writeAll( const unsigned char *buf, size_t len) = 0;

      /// writes len bytes from *buf to the socket
      /// returns the number of bytes actually written
      /// returns early if it could not write anyting for timeout miliseconds
      /// timeout equal to <0 means try for ever
      virtual size_t write( const unsigned char *buf, size_t len,
                            int timeout = 0 ) = 0;

      /// polls if the socket is available for
      /// reading or writing depending on mode
      /// times out after timeout milliseconds
      /// returns true if the socket is available
      virtual bool poll( PMode mode, int timeout ) = 0;

      /// closes the socket
      virtual void close() = 0;

      /// returns true if the socket is open
      virtual bool isOpen() const = 0;

      /// returns a string describing the socket
      virtual const std::string& desc() const = 0;

      /// cork the socket
      /// tells the socket only full packets should be put on the line.
      virtual void cork() {};

      /// uncork the socket
      /// send all remaining data in the buffer out and return to normal
      /// operation mode
      virtual void uncork() {};

      /// returns the associated certificate data if available.
      /// If no data is availabe it returns 0
      virtual const coral::ICertificateData* getCertificateData() const
      { return (coral::ICertificateData*) 0; };

      // temporary
      virtual int getFd() const = 0;

      virtual ~ISocket() {};
    };

    class SocketClosedException : public Exception
    {

    public:

      /// Constructor
      SocketClosedException( const std::string& methodName = "" )
        : Exception( "Socket is closed", methodName, "coral::CoralSockets" ) {}

      /// Destructor
      virtual ~SocketClosedException() throw() {}

    };


  }
}


#endif // CORALSOCKETS_TCPSOCKET_H
