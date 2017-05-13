// $Id: CoralServerFacade.h,v 1.31.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_CORALSERVERFACADE_H
#define CORALSERVER_CORALSERVERFACADE_H 1

// Include files
#include "CoralKernel/IHandle.h"
#include "CoralServerBase/ICoralFacade.h"
#include "RelationalAccess/../src/ICertificateConnectionService.h"

namespace coral
{

  namespace CoralServer
  {

    // Forward declaration
    class IObjectStoreMgr;
    class ITokenPool;

    /** @class CoralServerFacade
     *
     *  @author Andrea Valassi
     *  @date   2007-11-26 (python)
     *  @date   2008-01-22 (C++)
     */

    class CoralServerFacade : virtual public ICoralFacade
    {

    public:

      /// Constructor from an ITokenPool.
      /// The facade creates its own ConnectionService.
      CoralServerFacade( ITokenPool* tokenPool );

      /// Destructor.
      virtual ~CoralServerFacade();

      // --- FUNCTIONALITIES NEEDED IN READ-ONLY MODE ---

      /// Set the certificate data for this facade.
      /// The caller has to ensure that the ICertificateData pointer
      /// remains valid over the lifetime of the ICoralFacade.
      void setCertificateData( const coral::ICertificateData* cert )
      {
        m_certificateData = cert;
      };

      /// Get the sessionID for a new connection.
      /// The facade gets fromProxy=true if the request comes via a proxy.
      /// The client gets back fromProxy=true if the reply comes via a proxy.
      Token connect( const std::string& dbUrl,
                     const coral::AccessMode mode,
                     bool& fromProxy ) const;

      /// Release a session
      void releaseSession( Token sessionID ) const;

      /// Fetch the properties of a remote session
      const std::vector<std::string> fetchSessionProperties( Token sessionID ) const;

      /// Starts a transaction.
      void startTransaction( Token sessionID,
                             bool readOnly = false ) const;

      /// Commit a transaction.
      void commitTransaction( Token sessionID ) const;

      /// Rollback a transaction.
      void rollbackTransaction( Token sessionID ) const;

      /// Get the list of tables in a schema.
      const std::set<std::string> listTables( Token sessionID,
                                              const std::string& schemaName ) const;

      /// Check the existence of a table in a schema.
      bool existsTable( Token sessionID,
                        const std::string& schemaName,
                        const std::string& tableName ) const;

      /// Fetch the description of a table in a schema.
      const TableDescription fetchTableDescription( Token sessionID,
                                                    const std::string& schemaName,
                                                    const std::string& tableName ) const;

      /// Get the list of views in a schema.
      const std::set<std::string> listViews( Token sessionID,
                                             const std::string& schemaName ) const;

      /// Check the existence of a view in a schema.
      bool existsView( Token sessionID,
                       const std::string& schemaName,
                       const std::string& viewName ) const;

      /// Fetch the description and definition of a view in a schema.
      const std::pair<TableDescription,std::string> fetchViewDescription( Token sessionID,
                                                                          const std::string& schemaName,
                                                                          const std::string& viewName ) const;

      /// Fetch rows from a new query (a few at a time).
      /// This method is associated to a server-side open cursor (PULL model):
      /// the cursor is released when the loop ends or the iterator is deleted.
      /// This method must prefetch data into the iterator's internal cache:
      /// cache sizes are given in #rows (MB) if cacheSizeInMB is false (true).
      /// If given, the row buffer must be used by the iterator returning
      /// the results (and its specification must be shipped over the network
      /// to be used by the server-side IQuery::defineOutput call).
      IRowIteratorPtr fetchRows( Token sessionID,
                                 const QueryDefinition& qd,
                                 AttributeList* pOutputBuffer,
                                 size_t cacheSize,
                                 bool cacheSizeInMB ) const;

      /// Fetch rows from a new query (a few at a time).
      /// This method is associated to a server-side open cursor (PULL model):
      /// the cursor is released when the loop ends or the iterator is deleted.
      /// This method must prefetch data into the iterator's internal cache:
      /// cache sizes are given in #rows (MB) if cacheSizeInMB is false (true).
      /// The output type map must be shipped over the network to be used
      /// by the server-side IQuery::defineOutputTypes call).
      IRowIteratorPtr fetchRows( Token sessionID,
                                 const QueryDefinition& qd,
                                 const std::map< std::string, std::string > outputTypes,
                                 size_t cacheSize,
                                 bool cacheSizeInMB ) const;

      /// Fetch all rows from a new query.
      /// This method does NOT keep any server-side open cursors (PUSH model).
      /// If given, the row buffer must be used by the iterator returning
      /// the results (and its specification must be shipped over the network
      /// to be used by the server-side IQuery::defineOutput call).
      IRowIteratorPtr fetchAllRows( Token sessionID,
                                    const QueryDefinition& qd,
                                    AttributeList* pOutputBuffer ) const;

      /// Fetch all rows from a new query.
      /// This method does NOT keep any server-side open cursors (PUSH model).
      /// The output type map must be shipped over the network to be used
      /// by the server-side IQuery::defineOutputTypes call).
      IRowIteratorPtr fetchAllRows( Token sessionID,
                                    const QueryDefinition& qd,
                                    const std::map< std::string, std::string > outputTypes ) const;

      /// Execute a stored procedure with input parameters.
      void callProcedure( Token sessionID,
                          const std::string& schemaName,
                          const std::string& procedureName,
                          const coral::AttributeList& inputArguments ) const;

      // --- FUNCTIONALITIES NEEDED ONLY IN READ-WRITE MODE ---

      /// Delete rows from a table.
      /// Eventually pass an AttributeList.
      int deleteTableRows( Token sessionID,
                           const std::string& schemaName,
                           const std::string& tableName,
                           const std::string& whereClause,
                           const std::string& whereData ) const;

      /// Prepare a row buffer from the table metadata.
      /// Eventually retrieve an AttributeList.
      /// NB Eventually this will be replaced by other metadata methods.
      const std::string formatRowBufferAsString( Token sessionID,
                                                 const std::string& schemaName,
                                                 const std::string& tableName ) const;

      /// Insert one row.
      /// Eventually pass an AttributeList reference.
      void insertRowAsString( Token sessionID,
                              const std::string& schemaName,
                              const std::string& tableName,
                              const std::string& rowBufferAS ) const;

      /// Get the bulkOperationID for a new bulk insertion.
      /// Eventually pass an AttributeList reference.
      /// NB Is the rowBuffer here really needed or I got it wrong?
      Token bulkInsertAsString( Token sessionID,
                                const std::string& schemaName,
                                const std::string& tableName,
                                const std::string& rowBufferAS,
                                int rowCacheSizeDb ) const;

      /// Release a bulk operation
      void releaseBulkOp( Token bulkOpID ) const;

      /// Insert several rows in bulk.
      void processRows( Token bulkOpID,
                        const std::vector<AttributeList>& rows ) const;

      /// Flush a bulk operation.
      void flush( Token bulkOpID ) const;

    private:

      /// The CORAL connection service.
      /// [NB Should be mutable if ICoralFacade::connect() is const]
      IHandle<ICertificateConnectionService> m_pConnSvc;

      /// The object store manager (owned by this instance).
      IObjectStoreMgr* m_storeMgr;

      /// Pointer to the certificate data for this connection
      const coral::ICertificateData* m_certificateData;

    };

  }

}
#endif // CORALSERVER_CORALSERVERFACADE_H
