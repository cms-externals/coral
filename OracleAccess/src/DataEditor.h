#ifndef ORACLEACCESS_DATAEDITOR_H
#define ORACLEACCESS_DATAEDITOR_H 1

#include <boost/shared_ptr.hpp>
#include "RelationalAccess/ITableDataEditor.h"

namespace coral
{

  class ITableDescription;

  namespace OracleAccess
  {

    class SessionProperties;

    /**
     * Class DataEditor
     *
     * Implementation of the ITableDataEditor interface for the OracleAccess package
     */
    class DataEditor : virtual public coral::ITableDataEditor
    {
    public:
      /// Constructor
      DataEditor( boost::shared_ptr<const SessionProperties> properties,
                  const std::string& schemaName,
                  const coral::ITableDescription& description );

      /// Destructor
      virtual ~DataEditor() {}

      /**
       * Constructs a buffer corresponding to a full a table row.
       */
      void rowBuffer( coral::AttributeList& buffer );

      /**
       * Inserts a new row in the table.
       */
      void insertRow( const coral::AttributeList& dataBuffer );

      /**
       * Returns a new IOperationWithQuery object for performing an INSERT/SELECT operation
       */
      coral::IOperationWithQuery* insertWithQuery();

      /**
       * Returns a new IBulkOperation object for performing a bulk insert operation
       * specifying the input data buffer and the number of rows that should be cached on the client.
       */
      coral::IBulkOperation* bulkInsert( const coral::AttributeList& dataBuffer,
                                         int rowCacheSize );

      /**
       * Returns a new IBulkOperationWithQuery object for performing an INSERT/SELECT operation
       * specifying the number of iterations that should be cached on the client.
       */
      coral::IBulkOperationWithQuery* bulkInsertWithQuery( int dataCacheSize );

      /**
       * Updates rows in the table. Returns the number of affected rows.
       */
      long updateRows( const std::string& setClause,
                       const std::string& condition,
                       const coral::AttributeList& inputData );

      /**
       * Returns a new IBulkOperation object for performing a bulk update operation
       */
      coral::IBulkOperation* bulkUpdateRows( const std::string& setClause,
                                             const std::string& condition,
                                             const coral::AttributeList& inputData,
                                             int dataCacheSize );

      /**
       * Deletes the rows in the table fulfilling the specified condition. It returns the number of rows deleted.
       */
      long deleteRows( const std::string& condition,
                       const coral::AttributeList& conditionData );

      /**
       * Returns a new IBulkOperation for peforming a bulk delete operation.
       */
      coral::IBulkOperation* bulkDeleteRows( const std::string& condition,
                                             const coral::AttributeList& conditionData,
                                             int dataCacheSize );

    private:
      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this data editor
      const std::string m_schemaName;

      /// A reference to the table description object
      const coral::ITableDescription& m_tableDescription;
    };

  }

}

#endif
