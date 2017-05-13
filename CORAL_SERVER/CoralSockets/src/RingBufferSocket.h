// $Id: RingBufferSocket.h,v 1.6.2.2 2010/12/20 14:04:40 avalassi Exp $
#ifndef CORALSOCKETS_RINGBUFFERSOCKET_H
#define CORALSOCKETS_RINGBUFFERSOCKET_H 1

#include <sstream>

#include "CoralBase/boost_thread_headers.h"
#include <boost/shared_ptr.hpp>

#include "ISocket.h"

namespace coral {

  namespace CoralSockets {

    class RingBuffer {

    public:
      RingBuffer( size_t size );

      ~RingBuffer();

      /// reads at most len bytes from buf from the ringbuffer
      /// returns the number of byte actually read
      /// return -1 if buffer empty!
      int read( unsigned char * buf, size_t len );

      /// writes at most len bytes from buf into the ringbuffer
      /// returns the number of byte actually written
      /// return -1 if buffer full!
      int write( unsigned char * buf, size_t len );

      /// returns the used buffer size
      size_t usedSize();

      /// returns the free buffer size
      size_t freeSize()
      { return m_size - usedSize() -1; };


      void close()
      { m_isClosed = true; };

      bool isClosed()
      { return m_isClosed; };
    private:

      /// the size of the buffer
      size_t m_size;

      /// the buffer
      unsigned char *m_buffer;

      /// start of the data (reading end)
      unsigned int m_start;

      /// end of the data (writing end)
      unsigned int m_end;

      /// is it closed?
      bool m_isClosed;
    };
    // forward declaration
    class RingBufferSocket;

    typedef boost::shared_ptr<RingBufferSocket> RingBufferSocketPtr;

    class RingBufferSocket : virtual public ISocket {

      // this class serves as a drop in replacement for tcpsockets in tests

      friend class RingBufferPipes;
    public:
      // FIXME temporary?
      int getFd() const { return -1; };


      /// reads at most len bytes from the socket into *buf
      virtual size_t read( unsigned char *buf, size_t len,
                           int timeout = 0);

      /// writes len bytes from *buf to the socket
      virtual size_t write( const unsigned char *buf, size_t len,
                            int timeout = 0 );

      /// reads len bytes from the socket into *buf
      /// doesn't return until all bytes are read, or some error occurs
      virtual void readAll( unsigned char *buf, size_t len);

      /// writes len bytes into the socket into *buf
      /// doesn't return until all bytes are read, or some error occurs
      virtual void writeAll( const unsigned char *buf, size_t len);

      /// polls if the socket is available
      /// if timeout > 0 the request will timeout after timeout ms, timeout = 0
      /// means return immediately, < 0 wait for ever
      virtual bool poll( ISocket::PMode mode, int timeout );

      /// returns a string describing the socket
      virtual const std::string& desc() const
      { return m_desc; };

      /// closes the socket
      virtual void close() {
        m_write.close();
        m_read.close();
      };

      /// returns true if the socket is open
      virtual bool isOpen() const
      {
        return !m_write.isClosed();
      };

      virtual ~RingBufferSocket()
      {
        close();
      };

    private:
      RingBufferSocket( RingBuffer& read, boost::mutex & readMutex,
                        RingBuffer& write, boost::mutex & writeMutex,
                        const std::string& desc)
        : m_desc( desc )
        , m_read( read )
        , m_readMutex( readMutex )
        , m_write( write )
        , m_writeMutex( writeMutex )
      {};

      /// standard constructor is private
      RingBufferSocket();
      /// copy constructor is private
      RingBufferSocket(const RingBufferSocket& );
      /// assignment operator as well
      RingBufferSocket& operator=(const RingBufferSocket& );

      size_t read_write( PMode mode,
                         unsigned char *buf, size_t len, int timeout );


      /// short description of the connection
      const std::string m_desc;

      RingBuffer &m_read;
      boost::mutex &m_readMutex;

      RingBuffer &m_write;
      boost::mutex &m_writeMutex;
    };

    class RingBufferPipes {
    public:
      RingBufferPipes( size_t size, const std::string& desc )
        : m_forward( size )
        , m_forwardMutex()
        , m_backward( size )
        , m_backwardMutex()
        , m_src(  )
        , m_dst(  )
      {
        std::stringstream src_desc;
        src_desc << desc << " src";
        m_src = ISocketPtr( new RingBufferSocket(
                                                 m_forward, m_forwardMutex,
                                                 m_backward, m_backwardMutex,
                                                 src_desc.str() ) );

        std::stringstream dst_desc;
        dst_desc << desc << " dst";
        m_dst = ISocketPtr ( new RingBufferSocket(
                                                  m_backward, m_backwardMutex,
                                                  m_forward, m_forwardMutex,
                                                  dst_desc.str() ) );
      };


      ISocketPtr getSrc()
      { return m_src; };

      ISocketPtr getDst()
      { return m_dst; };

    private:
      RingBuffer m_forward;
      boost::mutex m_forwardMutex;

      RingBuffer m_backward;
      boost::mutex m_backwardMutex;

      ISocketPtr m_src;
      ISocketPtr m_dst;
    };
  }
}

#endif // CORALSOCKETS_TCPSOCKET_H
