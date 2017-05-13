// $Id: ICoralFacade.h,v 1.26.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVERBASE_ICORALFACADE_H
#define CORALSERVERBASE_ICORALFACADE_H 1

// Include files
#include <set>
#include <string>
#include <vector>
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/TableDescription.h"
#include "CoralServerBase/QueryDefinition.h"
#include "CoralServerBase/IRowIterator.h"

namespace coral
{

  class ICertificateData;

  /// ObjectID for sessionID, cursorID, bulkOpID.
  typedef unsigned int Token;

  /// Row iterator pointer.
  typedef std::auto_ptr<IRowIterator> IRowIteratorPtr;

  /** @class ICoralFacade
   *
   *  Interface to a facade to the CORAL relational functionalities.
   *  The methods of this interface represent ~all the messages exchanges
   *  required between the CORAL server and its clients over the network.
   *
   *  The special method fetchRows is an exception. The iterator returned
   *  by this method is an active object which internally sends additional
   *  requests to the server cursor (e.g. 'nextRows' or 'releaseCursor').
   *  The fetchRows method describes a PULL model for queries: rows are
   *  pulled in steps by the client from the server (an explicit 'nextRows'
   *  request by the client is needed at each step to fetch more rows).
   *  The fetchAllRows method describes a PUSH model for queries: all rows
   *  are pushed by the server towards the client at once.
   *  The advantage of fetchAllRows is that it does not keep open cursors;
   *  the disadvantage is that it requires more memory on the client to keep
   *  all rows sent by the server until the user reads them by looping.
   *
   *  @author Andrea Valassi and Alexander Kalkhof
   *  @date   2007-12-04
   */

  class ICoralFacade
  {

  public:

    /// Destructor.
    virtual ~ICoralFacade(){}

    // --- FUNCTIONALITIES NEEDED IN READ-ONLY MODE ---

    /// Set the certificate data for this facade.
    /// The caller has to ensure that the ICertificateData pointer
    /// remains valid over the lifetime of the ICoralFacade.
    virtual void setCertificateData( const coral::ICertificateData* cert ) = 0;

    /// Get the sessionID for a new connection.
    /// The facade gets fromProxy=true if the request comes via a proxy.
    /// The client gets back fromProxy=true if the reply comes via a proxy.
    virtual Token connect( const std::string& dbUrl,
                           const AccessMode mode,
                           bool& fromProxy ) const = 0;

    /// Release a session
    virtual void releaseSession( Token sessionID ) const = 0;

    /// Fetch the properties of a remote session
    virtual const std::vector<std::string> fetchSessionProperties( Token sessionID ) const = 0;

    /// Starts a transaction.
    virtual void startTransaction( Token sessionID,
                                   bool readOnly ) const = 0;

    /// Commit a transaction.
    virtual void commitTransaction( Token sessionID ) const = 0;

    /// Rollback a transaction.
    virtual void rollbackTransaction( Token sessionID ) const = 0;

    /// Get the list of tables in a schema.
    virtual const std::set<std::string> listTables( Token sessionID,
                                                    const std::string& schemaName ) const = 0;

    /// Check the existence of a table in a schema.
    virtual bool existsTable( Token sessionID,
                              const std::string& schemaName,
                              const std::string& tableName ) const = 0;

    /// Fetch the description of a table in a schema.
    virtual const TableDescription fetchTableDescription( Token sessionID,
                                                          const std::string& schemaName,
                                                          const std::string& tableName ) const = 0;

    /// Get the list of views in a schema.
    virtual const std::set<std::string> listViews( Token sessionID,
                                                   const std::string& schemaName ) const = 0;

    /// Check the existence of a view in a schema.
    virtual bool existsView( Token sessionID,
                             const std::string& schemaName,
                             const std::string& viewName ) const = 0;

    /// Fetch the description and definition of a view in a schema.
    virtual const std::pair<TableDescription,std::string> fetchViewDescription( Token sessionID,
                                                                                const std::string& schemaName,
                                                                                const std::string& viewName ) const = 0;

    /// Fetch rows from a new query (a few at a time).
    /// This method is associated to a server-side open cursor (PULL model):
    /// the cursor is released when the loop ends or the iterator is deleted.
    /// This method must prefetch data into the iterator's internal cache:
    /// cache sizes are given in #rows (MB) if cacheSizeInMB is false (true).
    /// If given, the row buffer must be used by the iterator returning
    /// the results (and its specification must be shipped over the network
    /// to be used by the server-side IQuery::defineOutput call).
    virtual IRowIteratorPtr fetchRows( Token sessionID,
                                       const QueryDefinition& qd,
                                       AttributeList* pOutputBuffer,
                                       size_t cacheSize,
                                       bool cacheSizeInMB ) const = 0;

    /// Fetch rows from a new query (a few at a time).
    /// This method is associated to a server-side open cursor (PULL model):
    /// the cursor is released when the loop ends or the iterator is deleted.
    /// This method must prefetch data into the iterator's internal cache:
    /// cache sizes are given in #rows (MB) if cacheSizeInMB is false (true).
    /// The output type map must be shipped over the network to be used
    /// by the server-side IQuery::defineOutputTypes call).
    virtual IRowIteratorPtr fetchRows( Token sessionID,
                                       const QueryDefinition& qd,
                                       const std::map< std::string, std::string > outputTypes,
                                       size_t cacheSize,
                                       bool cacheSizeInMB ) const = 0;

    /// Fetch all rows from a new query.
    /// This method does NOT keep any server-side open cursors (PUSH model).
    /// If given, the row buffer must be used by the iterator returning
    /// the results (and its specification must be shipped over the network
    /// to be used by the server-side IQuery::defineOutput call).
    virtual IRowIteratorPtr fetchAllRows( Token sessionID,
                                          const QueryDefinition& qd,
                                          AttributeList* pOutputBuffer ) const = 0;

    /// Fetch all rows from a new query.
    /// This method does NOT keep any server-side open cursors (PUSH model).
    /// The output type map must be shipped over the network to be used
    /// by the server-side IQuery::defineOutputTypes call).
    virtual IRowIteratorPtr fetchAllRows( Token sessionID,
                                          const QueryDefinition& qd,
                                          const std::map< std::string, std::string > outputTypes ) const = 0;

    /// Execute a stored procedure with input parameters.
    virtual void callProcedure( Token sessionID,
                                const std::string& schemaName,
                                const std::string& procedureName,
                                const coral::AttributeList& inputArguments ) const = 0;

    // --- FUNCTIONALITIES NEEDED ONLY IN READ-WRITE MODE ---

    /// Delete rows from a table.
    /// Eventually pass an AttributeList.
    virtual int deleteTableRows( Token sessionID,
                                 const std::string& schemaName,
                                 const std::string& tableName,
                                 const std::string& whereClause,
                                 const std::string& whereData ) const = 0;

    /// Prepare a row buffer from the table metadata.
    /// Eventually retrieve an AttributeList.
    /// NB Eventually this will be replaced by other metadata methods.
    virtual const std::string formatRowBufferAsString( Token sessionID,
                                                       const std::string& schemaName,
                                                       const std::string& tableName ) const = 0;

    /// Insert one row.
    /// Eventually pass an AttributeList reference.
    virtual void insertRowAsString( Token sessionID,
                                    const std::string& schemaName,
                                    const std::string& tableName,
                                    const std::string& rowBufferAS ) const = 0;

    /// Get the bulkOperationID for a new bulk insertion.
    /// Eventually pass an AttributeList reference.
    /// NB Is the rowBuffer here really needed or I got it wrong?
    virtual Token bulkInsertAsString( Token sessionID,
                                      const std::string& schemaName,
                                      const std::string& tableName,
                                      const std::string& rowBufferAS,
                                      int rowCacheSizeDb ) const = 0;

    /// Release a bulk operation
    virtual void releaseBulkOp( Token bulkOpID ) const = 0;

    /// Insert several rows in bulk.
    virtual void processRows( Token bulkOpID,
                              const std::vector<AttributeList>& rowsAS ) const = 0;

    /// Flush a bulk operation.
    virtual void flush( Token bulkOpID ) const = 0;

  };

}
#endif // CORALSERVERBASE_ICORALFACADE_H
