#ifndef SQLITEACCESS_BULKOPERATION_H
#define SQLITEACCESS_BULKOPERATION_H

#include "RelationalAccess/IBulkOperation.h"
#include <string>
#include <utility>
#include <vector>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

namespace coral {
  class AttributeList;
  namespace SQLiteAccess {
    class SessionProperties;
    /**
       @class BulkOperation BulkOperation.h
       Implementation of the IBulkOperation interface
       @author Zhen Xie
    */

    class BulkOperation : virtual public coral::IBulkOperation
    {
    public:
      /// Constructor
      BulkOperation(boost::shared_ptr<const SessionProperties> properties,
                    const coral::AttributeList& inputBuffer,
                    int cacheSize,
                    const std::string& statement );
      /// Destructor
      virtual ~BulkOperation();

      /**
       * Processes the next iteration
       */
      void processNextIteration();

      /**
       * Flushes the data on the client side to the server.
       */
      void flush();
    private:
      /// Resets the internal structures.
      void reset();
    private:

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_properties;

      /// The number of cached rows
      size_t m_rowCacheSize;
      /// The input row buffer
      const coral::AttributeList& m_inputRow;
      std::string m_sqlstatement;
      /// The actual data cache
      std::vector< std::vector< std::pair< boost::any, bool > >* > m_dataCache;
      /// Actual number of rows in the cache
      size_t m_rowsInCache;
    };
  } //ns SQLiteAccess
} //ns coral
#endif
