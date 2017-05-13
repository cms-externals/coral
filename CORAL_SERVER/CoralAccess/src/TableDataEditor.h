// $Id: TableDataEditor.h,v 1.1.2.1 2010/12/20 09:10:09 avalassi Exp $
#ifndef CORALACCESS_ITABLEDATAEDITOR_H
#define CORALACCESS_ITABLEDATAEDITOR_H 1

// Include files
#include <string>
#include "CoralServerBase/NotImplemented.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITableDescription.h"

namespace coral
{

  namespace CoralAccess
  {

    // Forward declaration
    class SessionProperties;

    /** @class TableDataEditor
     *
     *  @author Andrea Valassi
     *  @date   2009-01-30
     */

    class TableDataEditor : virtual public ITableDataEditor
    {

    public:

      /// Constructor
      TableDataEditor( const SessionProperties& sessionProperties,
                       const ITableDescription& tableDescription );

      /// Destructor
      ~TableDataEditor();

      /**
       * Constructs a buffer corresponding to a full a table row.
       */
      void rowBuffer( AttributeList& buffer );

      /**
       * Inserts a new row in the table.
       */
      void insertRow( const AttributeList& /*dataBuffer*/ )
      {
        throw NotImplemented( "TableDataEditor::insertRow" );
      }

      /**
       * Returns a new IOperationWithQuery object for performing an INSERT/SELECT operation
       */
      IOperationWithQuery* insertWithQuery()
      {
        throw NotImplemented( "TableDataEditor::insertWithQuery" );
      }

      /**
       * Returns a new IBulkOperation object for performing a bulk insert operation
       * specifying the input data buffer and the number of rows that should be cached on the client.
       */
      IBulkOperation* bulkInsert( const AttributeList& /*dataBuffer*/,
                                  int /*rowCacheSize*/ )
      {
        throw NotImplemented( "TableDataEditor::bulkInsert" );
      }

      /**
       * Returns a new IBulkOperationWithQuery object for performing an INSERT/SELECT operation
       * specifying the number of iterations that should be cached on the client.
       */
      IBulkOperationWithQuery* bulkInsertWithQuery( int /*dataCacheSize*/ )
      {
        throw NotImplemented( "TableDataEditor::insertRow" );
      }

      /**
       * Updates rows in the table. Returns the number of affected rows.
       */
      long updateRows( const std::string& /*setClause*/,
                       const std::string& /*condition*/,
                       const AttributeList& /*inputData*/ )
      {
        throw NotImplemented( "TableDataEditor::updateRows" );
      }

      /**
       * Returns a new IBulkOperation object for performing a bulk update operation
       */
      IBulkOperation* bulkUpdateRows( const std::string& /*setClause*/,
                                      const std::string& /*condition*/,
                                      const AttributeList& /*inputData*/,
                                      int /*dataCacheSize*/ )
      {
        throw NotImplemented( "TableDataEditor::bulkUpdateRows" );
      }

      /**
       * Deletes the rows in the table fulfilling the specified condition. It returns the number of rows deleted.
       */
      long deleteRows( const std::string& /*condition*/,
                       const AttributeList& /*conditionData*/ )
      {
        throw NotImplemented( "TableDataEditor::deleteRows" );
      }

      /**
       * Returns a new IBulkOperation for peforming a bulk delete operation.
       */
      IBulkOperation* bulkDeleteRows( const std::string& /*condition*/,
                                      const AttributeList& /*conditionData*/,
                                      int /*dataCacheSize*/ )
      {
        throw NotImplemented( "TableDataEditor::bulkDeleteRows" );
      }

    private:

      /// Session properties reference.
      const SessionProperties& m_sessionProperties;

      /// Table description reference
      const ITableDescription& m_tableDescription;

    };
  }
}
#endif // CORALACCESS_TABLEDATAEDITOR_H
