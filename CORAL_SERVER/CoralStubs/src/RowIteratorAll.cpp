#include "RowIteratorAll.h"
#include "Exceptions.h"

#include "CALProtocol.h"

//Include logger
#define LOGGER_NAME "CoralStubs::RowIteratorAll"
#include "logger.h"

#include <iostream>

namespace coral { namespace CoralStubs {

  RowIteratorAll::RowIteratorAll(IByteBufferIterator* reply, AttributeList* rowBuffer, CALOpcode opcode)
    : m_obuffer( rowBuffer )
    , m_cbuffer( false )
    , m_ibuffer( false )
    , m_fbuffer( true )
    , m_reply( reply )
    , m_sri( new SegmentReaderIterator( opcode, *reply ) )
    , m_structure( 0 )
  {
    if(m_obuffer)
    {
      m_fbuffer = m_obuffer->size() == 0;
    }
    else
    {
      //create a new attribute list
      m_obuffer = new AttributeList;
      m_cbuffer = true;
    }
  }

  RowIteratorAll::~RowIteratorAll()
  {
    if(m_cbuffer) delete m_obuffer;
    delete m_sri;
    delete m_reply;
    free(m_structure);
  }

  bool
  RowIteratorAll::next()
  {
    bool hasnext;
    m_sri->extract( hasnext );
    if(hasnext)
    {
      if(m_structure)
      {
        //get only the raw data
        m_sri->extractD( m_structure );

        m_ibuffer = true;
        return true;
      }
      else
      {
        if(m_fbuffer)
        {
          if(m_obuffer->size() > 0)
            throw ReplyIteratorException("AttributeList structure was changed during request and next", "RowIteratorAll::next()");
          //fill up the attribute structure
          //in the case that the attributelist was empty
          m_sri->extractE( *m_obuffer );
          m_fbuffer = false;
        }
        //now we have a structure in the buffer
        m_structure = m_sri->getStructure( *m_obuffer );
        //get only the raw data
        m_sri->extractD( m_structure );

        m_ibuffer = true;
        return true;
      }
    }
    else
    {
      m_ibuffer = false;
      return false;
    }
  }

  const AttributeList&
  RowIteratorAll::currentRow() const
  {
    if(!m_ibuffer)
      throw ReplyIteratorException("no AttributeList available, next() was not called", "RowIteratorAll::currentRow()");

    return *m_obuffer;
  }

} }
