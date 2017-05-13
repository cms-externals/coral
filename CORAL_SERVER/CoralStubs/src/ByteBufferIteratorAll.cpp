#include "ByteBufferIteratorAll.h"

namespace coral { namespace CoralStubs {

  ByteBufferIteratorAll::ByteBufferIteratorAll(IRowIterator* rowi, CALOpcode opcode, bool cacheable, bool proxy, bool isempty, AttributeList* rowBuffer)
    : m_swi(opcode, cacheable, true)
    , m_rowi( rowi )
    , m_rowBuffer( rowBuffer )
    , m_isempty( isempty )
    , m_structure( 0 )
    , m_islast( false )
    , m_lastbuffer( false )
  {
    m_swi.setProxy( proxy );

    if(isempty)
    {
      if(m_rowi->next())
      {
        m_swi.append( true );

        const AttributeList& attr01 = m_rowi->currentRow();
        //first send the attribute list
        m_swi.appendE( attr01 );

        m_structure = m_swi.getStructure( attr01 );

        //write only the raw data
        m_swi.appendD( m_structure );
        /*
        while(m_rowi->next())
        {
          const AttributeList& attr02 = m_rowi->currentRow();

          m_swi.append( true );
          //write only the raw data
          m_swi.appendD( m_structure, attr02 );
        }
        */
      }
      else
      {
        //send termination
        m_swi.append( false );
        m_swi.flush();
        //set as last block
        m_islast = true;
      }

      //      m_swi.append( false );
    }
    else
    {
      //rowbuffer remains always the same
      //so we can use the rowbuffer as pointer
      m_structure = m_swi.getStructure( *m_rowBuffer );
      /*
      while(m_rowi->next())
      {
        m_swi.append( true );
        //write only the raw data
        m_swi.appendD( m_structure );
      }
      m_swi.append( false );
      */
    }
    //    m_swi.flush();
  }

  ByteBufferIteratorAll::~ByteBufferIteratorAll()
  {
    if(m_rowBuffer) delete m_rowBuffer;
    delete m_rowi;
    if(m_structure) free( m_structure );
  }

  void
  ByteBufferIteratorAll::fillBuffer()
  {
    while(!m_swi.next())
    {
      bool newrow = m_rowi->next();
      if(newrow)
      {
        if(m_isempty)
        {
          const AttributeList& attr02 = m_rowi->currentRow();

          m_swi.append( true );
          //write only the raw data
          m_swi.appendD( m_structure, attr02 );
        }
        else
        {
          m_swi.append( true );
          //write only the raw data
          m_swi.appendD( m_structure );
        }
      }
      else
      {
        //set the terminal
        m_swi.append( false );
        //flush the last time
        //now we should have something in the buffer
        //will return in the next loop
        m_swi.flush();
        //return as is last
        m_islast = true;
      }
    }
    //check if it was the last
    //only check if the m_islast set to true
    //otherwise it is always last as long there is only one entry in the buffer
    //FIXME fix that on the swi
    m_lastbuffer = m_swi.isLast() && m_islast;
  }

  bool
  ByteBufferIteratorAll::next()
  {
    if(!m_islast)
    {
      if(m_swi.next())
        return true;
      else
      {
        fillBuffer();
        return true;
      }
    }
    else
    {
      if(m_swi.next())
      {
        m_lastbuffer = m_swi.isLast();
        return true;
      }
    }
    return false;
  }

  bool
  ByteBufferIteratorAll::isLast() const
  {
    return m_lastbuffer;
  }

  const ByteBuffer&
  ByteBufferIteratorAll::currentBuffer() const
  {
    return m_swi.currentBuffer();
  }


} }
