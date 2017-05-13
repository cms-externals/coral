// $Id: RowVectorIterator.cpp,v 1.6.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include "CoralBase/Attribute.h"
#include "CoralBase/Exception.h"
//#include "CoralServerBase/attributeListToString.h"

// Local include files
#include "RowVectorIterator.h"

// Logger
#define LOGGER_NAME "CoralServer::RowVectorIterator"
#include "logger.h"

// Namespace
using namespace coral::CoralServer;

//-----------------------------------------------------------------------------

inline void i_checkSpec( const std::string& methodName,
                         const coral::AttributeList& expSpec,
                         const coral::AttributeList& spec )
{
  if ( expSpec.size() != spec.size() )
  {
    logger << coral::Error << "Row buffer specification mismatch (#attributes)" << endlog;
    logger << coral::Error << "Expected spec: " << expSpec << endlog;
    logger << coral::Error << "Current  spec: " << spec << endlog;
    throw coral::Exception( "Row buffer specification mismatch (#attributes)",
                            methodName,
                            "coral::CoralServer" );
  }
  for ( unsigned int iAttr = 0; iAttr < expSpec.size(); iAttr++ )
  {
    if ( expSpec[iAttr].specification() != spec[iAttr].specification() )
      throw coral::Exception( "Row buffer specification mismatch (attribute spec)",
                              methodName,
                              "coral::CoralServer" );
  }
}

//-----------------------------------------------------------------------------

RowVectorIterator::RowVectorIterator( AttributeList* pBuffer,
                                      const std::vector<AttributeList> rows )
  : m_next( 0 ) // not started
  , m_hasBuffer( pBuffer )
  , m_pBuffer( pBuffer )
  , m_rows( rows )
{
  //logger << Always << "Create RowVectorIterator..." << endlog;
  if ( pBuffer )
  {
    for ( unsigned int i=0; i<m_rows.size(); i++ )
      i_checkSpec( "RowVectorIterator::RowVectorIterator", *pBuffer, m_rows[i] );
  }
  //logger << Always << "Create RowVectorIterator... DONE" << endlog;
}

//-----------------------------------------------------------------------------

RowVectorIterator::~RowVectorIterator()
{
}

//-----------------------------------------------------------------------------

bool RowVectorIterator::next()
{
  if ( m_next < m_rows.size() ) // m_next was [0,n-1]
  {
    if ( m_hasBuffer )
    {
      //*(m_pBuffer) = m_rows[m_next]; // BUG!!!
      m_pBuffer->copyData( m_rows[m_next] ); // BUG FIX
      i_checkSpec( "RowVectorIterator::next", *m_pBuffer, m_rows[m_next] );
      //logger << "RowVectorIterator::next() WITH hasBuffer\n"
      //       << &(m_rows[m_next]) << ": " << attributeListToString( m_rows[m_next] ) << "\n"
      //       << m_pBuffer << ": " << attributeListToString( *(m_pBuffer) ) << endlog;
      //logger << "RowVectorIterator::next() Attribute#0 is " << &((*m_pBuffer)[0]) << endlog;
    }
    else
    {
      m_pBuffer = const_cast<AttributeList*>( &(m_rows[m_next]) );
      //logger << "RowVectorIterator::next() WITHOUT hasBuffer\n"
      //       << &(m_rows[m_next]) << ": " << attributeListToString( m_rows[m_next] ) << "\n"
      //       << m_pBuffer << ": " << attributeListToString( *(m_pBuffer) ) << endlog;
      //logger << "RowVectorIterator::next() Attribute#0 is " << &((*m_pBuffer)[0]) << endlog;
    }
    m_next++;
    return true; // m_next is now [1,n]
  }
  else if ( m_next == m_rows.size() ) // m_next was n
  {
    m_next++;
    return false; // m_next is now n+1
  }
  else if ( m_next == m_rows.size()+1 ) // m_next was n+1
  {
    return false; // m_next is still n+1
  }
  else
  {
    throw Exception( "PANIC! Iterator is in invalid state!",
                     "RowVectorIterator::next",
                     "coral::CoralServer" );
  }
}

//-----------------------------------------------------------------------------

bool RowVectorIterator::isLast() const
{
  if ( m_next == 0 )
    throw Exception( "Iterator is still before the start",
                     "RowVectorIterator::isLast",
                     "coral::CoralServer" );
  else if ( m_next < m_rows.size() )
    return false;  // m_next is [1,n-1]
  else if ( m_next == m_rows.size() )
    return true;  // m_next is [n]
  else if ( m_next == m_rows.size()+1 )
    throw Exception( "Iterator is already past the end",
                     "RowVectorIterator::isLast",
                     "coral::CoralServer" );
  else
    throw Exception( "PANIC! Iterator is in invalid state!",
                     "RowVectorIterator::isLast",
                     "coral::CoralServer" );
}

//-----------------------------------------------------------------------------

const coral::AttributeList& RowVectorIterator::currentRow() const
{
  if ( m_next == 0 )
    throw Exception( "Iterator is still before the start",
                     "RowVectorIterator::currentRow",
                     "coral::CoralServer" );
  else if ( m_next <= m_rows.size() )
    return *m_pBuffer;  // m_next is [1,n]
  else if ( m_next == m_rows.size()+1 )
    throw Exception( "Iterator is already past the end",
                     "RowVectorIterator::currentRow",
                     "coral::CoralServer" );
  else
    throw Exception( "PANIC! Iterator is in invalid state!",
                     "RowVectorIterator::currentRow",
                     "coral::CoralServer" );
}

//-----------------------------------------------------------------------------
