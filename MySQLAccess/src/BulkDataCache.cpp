// $Id: BulkDataCache.cpp,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <iostream>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/AttributeSpecification.h"

#include "BulkDataCache.h"

namespace coral
{
  namespace MySQLAccess
  {
    BulkDataCache::BulkDataCache( const coral::AttributeList& input, size_t size /*=100*/ )
      : m_input( input )
      , m_size( 0 )
      , m_maxSize( size )
      , m_specification( 0 )
      , m_data()
    {
      reset();
    }

    BulkDataCache::~BulkDataCache()
    {
      if( m_specification )
      {
        m_specification->release(); m_specification = 0;
      }
      m_data.clear();
    }

    size_t BulkDataCache::size() const
    {
      return m_size;
    }

    bool BulkDataCache::empty() const
    {
      return m_data.empty();
    }

    bool BulkDataCache::full() const
    {
      return( m_maxSize == m_size );
    }

    void BulkDataCache::insert()
    {
      insert( m_input );
    }

    void BulkDataCache::insert( const coral::AttributeList& input )
    {
      if( ! full() )
      {
        m_data[m_size++].fastCopyData( input );
      }
    }

    const coral::AttributeList& BulkDataCache::row( size_t index ) const
    {
      return m_data[index];
    }

    void BulkDataCache::clear()
    {
      m_size = 0;
    }

    void BulkDataCache::reset()
    {
      if( !empty() )
      {
        m_data.clear();
        m_size = 0;
      }

      m_data.reserve( m_maxSize );

      // Prepare an empty specification
      m_specification = new coral::AttributeListSpecification();

      for( coral::AttributeList::iterator ai = m_input.begin(); ai != m_input.end(); ++ai )
      {
        m_specification->extend( (*ai).specification().name(), (*ai).specification().type() );
      }

      // Fill the data cache with empty rows
      for( size_t i = 0; i < m_maxSize; i++ )
        m_data.push_back( coral::AttributeList( const_cast<const coral::AttributeListSpecification&>(*m_specification), true ) );

      // Release the specification, not needed for data cache anymore
      m_specification->release(); m_specification = 0;
    }
  }
}
