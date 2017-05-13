// $Id: BulkOperation.h,v 1.4 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_BUILK_OPERATION_H
#define MYSQLACCESS_BUILK_OPERATION_H 1

#include <string>
#include <vector>
#include "RelationalAccess/IBulkOperation.h"

#include "BulkDataCache.h"

namespace coral
{

  class AttributeList;

  namespace MySQLAccess
  {

    class ISessionProperties;
    class Statement;

    /**
     * Class BulkOperation
     *
     * Implementation of the IBulkOperation interface
     */

    class BulkOperation : virtual public coral::IBulkOperation
    {
    public:
      /// Constructor
      BulkOperation( const ISessionProperties& properties, const coral::AttributeList& inputBuffer, int cacheSize, const std::string& statement );

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
      /// Resets the operation and closes the statement handle
      void reset();

    private:
      /// A reference to the sessopm properties
      const ISessionProperties&         m_sessionProperties;
      /// The OCI statement handle
      coral::MySQLAccess::Statement*    m_statement;
      /// Data buffer
      coral::MySQLAccess::BulkDataCache m_data;
    };

  }

}

#endif
