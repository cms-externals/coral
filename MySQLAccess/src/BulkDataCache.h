// $Id: BulkDataCache.h,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_BULKDATACACHE_H
#define MYSQLACCESS_BULKDATACACHE_H 1

#include <vector>

namespace coral
{
  class AttributeList;
  class AttributeListSpecification;

  namespace MySQLAccess
  {
    /**
     * Class BulkDataCache
     * Transient data buffer for bulk records to be applied by bulk operations
     * like INSERT, UPDATE or DELETE, eventually for queries
     */
    class BulkDataCache
    {
    public:
      /// Constructor
      BulkDataCache( const coral::AttributeList& input, size_t size=100 );
      /// Destructor
      ~BulkDataCache();
      /// Number of rows in the cache
      size_t size() const;
      /// True if no rows in the cache
      bool empty() const;
      /// True is number of rows reached the initially required size
      bool full() const;
      /// Insert a new data row into the cache
      void insert();
      void insert( const coral::AttributeList& input );
      /// Get the i-th row from the data cache
      const coral::AttributeList& row( size_t index ) const;
      /// Light reset
      void clear();
      /// Reset the data cache contents
      void reset();
      void reset( const coral::AttributeList& input, size_t size=100 );

    private:
      const coral::AttributeList&         m_input;
      size_t m_size;
      size_t m_maxSize;
      coral::AttributeListSpecification*  m_specification;
      std::vector<coral::AttributeList>   m_data;
    };
  }
}

#endif // MYSQLACCESS_BULKDATACACHE_H
