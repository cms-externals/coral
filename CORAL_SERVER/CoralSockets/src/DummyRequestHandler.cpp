// $Id: DummyRequestHandler.cpp,v 1.8.2.2 2011/09/16 16:06:14 avalassi Exp $

// Include files
#include <cstring> // For memcpy
#include <sstream>
#include "CoralSockets/GenericSocketException.h"
#include "DummyRequestHandler.h"

// Namespace
using namespace coral;
using namespace coral::CoralSockets;

//-----------------------------------------------------------------------------

SimpleReplyIterator::SimpleReplyIterator( ByteBuffer& buffer, bool isLast )
  : IByteBufferIterator()
  , m_currentBuffer( (ByteBuffer*) 0 )
  , m_buffers( )
  , m_gotLastBuffer( isLast )
{
  boost::shared_ptr<ByteBuffer> newBuf( new ByteBuffer( buffer.usedSize() ) );
  for (size_t i=0; i< buffer.usedSize(); i++)
    *(newBuf->data()+i) = *(buffer.data()+i);
  newBuf->setUsedSize( buffer.usedSize() );
  m_buffers.push( newBuf );
}

//-----------------------------------------------------------------------------

SimpleReplyIterator::SimpleReplyIterator( boost::shared_ptr<ByteBuffer> buffer, bool isLast )
  : IByteBufferIterator()
  , m_currentBuffer( (ByteBuffer*) 0 )
  , m_buffers( )
  , m_gotLastBuffer( isLast )
{
  m_buffers.push( buffer );
}

//-----------------------------------------------------------------------------

void SimpleReplyIterator::addBuffer( ByteBuffer& buffer, bool isLast)
{
  boost::shared_ptr<ByteBuffer> newBuf( new ByteBuffer( buffer.usedSize() ) );
  for (size_t i=0; i< buffer.usedSize(); i++)
    *(newBuf->data()+i) = *(buffer.data()+i);
  newBuf->setUsedSize( buffer.usedSize() );

  addBuffer( newBuf, isLast );
}

//-----------------------------------------------------------------------------

void SimpleReplyIterator::addBuffer( boost::shared_ptr<ByteBuffer> buffer, bool isLast)
{
  if ( m_gotLastBuffer )
    throw GenericSocketException("SimpleReplyIterator already as got the last"
                                 " buffer, you can't add more!", "SimpleReplyIterator::addBuffer");

  m_buffers.push( buffer );
  m_gotLastBuffer=isLast;
}

//-----------------------------------------------------------------------------

bool SimpleReplyIterator::next()
{
  // the class is not thread safe
  if (!m_gotLastBuffer)
    throw GenericSocketException("can't call next() before all buffers are "
                                 "added!","SimpleReplyIterator::next()");

  if ( m_buffers.empty() && !m_gotLastBuffer )
    throw GenericSocketException("m_buffers is empty, m_gotLastBuffer is false",
                                 "SimpleReplyIterator::next()");

  if ( m_buffers.empty() ) {
    m_currentBuffer = boost::shared_ptr<ByteBuffer>( (ByteBuffer*)0 );
    return false;
  };

  m_currentBuffer=m_buffers.front();
  m_buffers.pop();

  return true;
}

//-----------------------------------------------------------------------------

bool SimpleReplyIterator::isLast() const
{
  if ( m_currentBuffer.get() == 0 )
    throw GenericSocketException("no current buffer, did you call next()?",
                                 "SimpleReplyIterator::next()");

  return (m_buffers.empty() && m_gotLastBuffer );
}

//-----------------------------------------------------------------------------

const ByteBuffer& SimpleReplyIterator::currentBuffer() const
{
  if (m_currentBuffer.get() == 0 )
    throw GenericSocketException("no current buffer, did you call next()?",
                                 "SimpleReplyIterator::next()");

  return *m_currentBuffer;
}

//-----------------------------------------------------------------------------

DummyRequestHandler::DummyRequestHandler()
{
}

//-----------------------------------------------------------------------------

DummyRequestHandler::~DummyRequestHandler()
{
}

//-----------------------------------------------------------------------------

void
DummyRequestHandler::setCertificateData( const coral::ICertificateData* /*cert*/ )
{
  // DUMMY
}

//-----------------------------------------------------------------------------

IByteBufferIteratorPtr
DummyRequestHandler::replyToRequest( const ByteBuffer& request )
{
  std::string requestStr( (char*)request.data(), request.usedSize() );
  if (requestStr.find("sleep ") == 0 ) {
    // sleep before reply requested
    unsigned int num=0;
    std::stringstream numStream( requestStr.substr( 5, requestStr.size() ) );
    numStream >> num;
    if (num>100)
      num=100;
    sleep( num );
  };

  if (requestStr.find("copy ") == 0 ) {
    // multi ByteBuffer reply requested
    unsigned int num=0;
    std::stringstream numStream( requestStr.substr( 5, requestStr.size() ) );
    numStream >> num;
    if (num>10)
      num=10;

    unsigned int count=0;
    std::unique_ptr<SimpleReplyIterator> iterator;
    do {
      std::stringstream stream;
      stream <<"Thank you for your request " << count << " '";
      std::string thanks( stream.str() );

      ByteBuffer reply( thanks.size() + request.usedSize() );
      ::memcpy( reply.data(), thanks.data(), thanks.size() );
      ::memcpy( reply.data() + thanks.size(), request.data(),
                request.usedSize() );
      reply.setUsedSize( thanks.size() + request.usedSize() );

      count++;
      if ( iterator.get() == 0 )
        iterator = std::unique_ptr<SimpleReplyIterator>(
                                                      new SimpleReplyIterator( reply, false ) );
      else
        iterator->addBuffer( reply, count>=num );

    } while ( count < num );

    return (IByteBufferIteratorPtr)iterator;
  }

  std::string thanks = "Thank you for your request '";
  ByteBuffer reply( thanks.size() + request.usedSize() );;
  ::memcpy( reply.data(), thanks.data(), thanks.size() );
  ::memcpy( reply.data() + thanks.size(), request.data(), request.usedSize() );
  reply.setUsedSize( thanks.size() + request.usedSize() );
  return IByteBufferIteratorPtr( new SimpleReplyIterator( reply ) );
}

//-----------------------------------------------------------------------------
