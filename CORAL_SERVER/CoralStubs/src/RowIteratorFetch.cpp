#include "RowIteratorFetch.h"

//CoralStubs includes
#include "CALProtocol.h"
#include "AttributeUtils.h"
#include "Exceptions.h"
#include "SegmentWriterIterator.h"
#include "SegmentReaderIterator.h"

//CoralServerBase includes

//Include logger
#define LOGGER_NAME "CoralStubs::RowIteratorFetch"
#include "logger.h"

//C++ includes
#include <iostream>
#include <sstream>

namespace coral { namespace CoralStubs {

  RowIteratorFetch::RowIteratorFetch(IRequestHandler& requestHandler, uint32_t cursorID, size_t maxrows, AttributeList* rowBuffer)
    : m_requestHandler( requestHandler )
    , m_cursor( cursorID )
    , m_maxrows( maxrows )
    , m_obuffer( rowBuffer )
    , m_iterbuffer( 0 )
    , m_waslast( false )
    , m_requestno( 0 )
  {
  }

  RowIteratorFetch::~RowIteratorFetch()
  {
    std::list< AttributeList* >::iterator i;
    for(i = m_buffers.begin(); i != m_buffers.end(); i++ )
    {
      delete *i;
    }

    if(m_iterbuffer) delete m_iterbuffer;
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::ReleaseCursor, true, false );

    swi.append( m_cursor );
    //call the request handler and keep the reply interator
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );

    SegmentReaderIterator sri( coral::CALOpcodes::ReleaseCursor, *(reply.get()) );

    sri.noextract();
  }

  void
  RowIteratorFetch::fillBuffer()
  {
    m_requestno++;
    //create the request buffer object as cacheable and no reply
    SegmentWriterIterator swi( coral::CALOpcodes::FetchRowsNext, true, false );

    swi.append( m_cursor );
    //call the request handler and keep the reply interator
    coral::IByteBufferIteratorPtr reply = m_requestHandler.replyToRequest( swi.singleBuffer() );

    SegmentReaderIterator sri( coral::CALOpcodes::FetchRowsNext, *(reply.get()) );

    size_t rows = 0;

    bool hasnext;
    sri.extract( hasnext );

    while( hasnext )
    {
      //create a new instance of an attribute list
      AttributeList* atlptr = new AttributeList;
      //read the list from the stream buffer
      sri.extractV( *atlptr );
      //add the list to the buffer
      m_buffers.push_back(atlptr);
      //read if there is a next
      sri.extract( hasnext );

      rows++;
    }
    //read out the is last bool
    sri.extract( m_waslast );
  }

  bool
  RowIteratorFetch::next()
  {
    if(m_buffers.size() > 0)
    {
      if(m_iterbuffer) delete m_iterbuffer;

      m_iterbuffer = m_buffers.front();

      m_buffers.pop_front();

      if(m_obuffer)
      {
        if(m_obuffer->size() == 0)
        {
          //if we have an empty attribute list
          //we must first copy the structure of it
          copyAttributeListsEntries(*m_obuffer, *m_iterbuffer );
        }
        //fills the attribute list structure with values
        //checks if the both attribute lists are equal
        copyAttributeLists(*m_obuffer, *m_iterbuffer );
      }

      return true;
    }

    if(m_waslast)
    {
      if(m_iterbuffer) delete m_iterbuffer;

      m_iterbuffer = 0;

      return false;
    }

    if(m_iterbuffer) delete m_iterbuffer;

    m_iterbuffer = 0;

    fillBuffer();

    return next();
  }

  const AttributeList&
  RowIteratorFetch::currentRow() const
  {
    if(!m_iterbuffer)
      throw ReplyIteratorException("no AttributeList available", "RowIteratorAll::currentRow()");

    if(m_obuffer)
      return *m_obuffer;

    return *m_iterbuffer;
  }

} }
