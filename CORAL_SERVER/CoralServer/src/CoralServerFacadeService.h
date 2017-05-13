// $Id: CoralServerFacadeService.h,v 1.26.2.1 2010/12/20 09:10:10 avalassi Exp $
#ifndef CORALSERVER_CORALSERVERFACADESERVICE_H
#define CORALSERVER_CORALSERVERFACADESERVICE_H 1

// Include files
#include "CoralKernel/Service.h"
#include "CoralServerBase/ICoralFacade.h"
#include "CoralServer/CoralServerFacade.h"

namespace coral
{

  namespace CoralServer
  {

    /** @class CoralServerFacadeService
     *
     *  Wrapper around a CoralServerFacade.
     *  This is a plugin that can be loaded at runtime.
     *
     *  @author Andrea Valassi
     *  @date   2009-01-30
     */

    class CoralServerFacadeService : public Service,
                                     virtual public ICoralFacade
    {

    public:

      /// Service constructor from the service name.
      CoralServerFacadeService( const std::string& key );

      /// Destructor.
      virtual ~CoralServerFacadeService();

      // --- FUNCTIONALITIES NEEDED IN READ-ONLY MODE ---

      /// Set the certificate data for this facade.
      /// The caller has to ensure that the ICertificateData pointer
      /// remains valid over the lifetime of the ICoralFacade.
      void setCertificateData( const coral::ICertificateData* cert )
      {
        m_facade.setCertificateData( cert );
      }

      /// Get the sessionID for a new connection.
      /// The facade gets fromProxy=true if the request comes via a proxy.
      /// The client gets back fromProxy=true if the reply comes via a proxy.
      Token connect( const std::string& dbUrl,
                     const AccessMode mode,
                     bool& fromProxy) const
      {
        return m_facade.connect( dbUrl, mode, fromProxy );
      }

      /// Release a session
      void releaseSession( Token sessionID ) const
      {
        return m_facade.releaseSession( sessionID );
      }

      /// Fetch the properties of a remote session
      const std::vector<std::string> fetchSessionProperties( Token sessionID ) const
      {
        return m_facade.fetchSessionProperties( sessionID );
      }

      /// Starts a transaction.
      void startTransaction( Token sessionID,
                             bool readOnly ) const
      {
        return m_facade.startTransaction( sessionID, readOnly );
      }

      /// Commit a transaction.
      void commitTransaction( Token sessionID ) const
      {
        return m_facade.commitTransaction( sessionID );
      }

      /// Rollback a transaction.
      void rollbackTransaction( Token sessionID ) const
      {
        return m_facade.rollbackTransaction( sessionID );
      }

      /// Get the list of tables in a schema.
      const std::set<std::string> listTables( Token sessionID,
                                              const std::string& schemaName ) const
      {
        return m_facade.listTables( sessionID, schemaName );
      }

      /// Check the existence of a table in a schema.
      bool existsTable( Token sessionID,
                        const std::string& schemaName,
                        const std::string& tableName ) const
      {
        return m_facade.existsTable( sessionID, schemaName, tableName );
      }

      /// Fetch the description of a table in a schema.
      const TableDescription fetchTableDescription( Token sessionID,
                                                    const std::string& schemaName,
                                                    const std::string& tableName ) const
      {
        return m_facade.fetchTableDescription( sessionID, schemaName, tableName );
      }

      /// Get the list of views in a schema.
      const std::set<std::string> listViews( Token sessionID,
                                             const std::string& schemaName ) const
      {
        return m_facade.listViews( sessionID, schemaName );
      }

      /// Check the existence of a view in a schema.
      bool existsView( Token sessionID,
                       const std::string& schemaName,
                       const std::string& viewName ) const
      {
        return m_facade.existsView( sessionID, schemaName, viewName );
      }

      /// Fetch the description and definition of a view in a schema.
      const std::pair<TableDescription,std::string> fetchViewDescription( Token sessionID,
                                                                          const std::string& schemaName,
                                                                          const std::string& viewName ) const
      {
        return m_facade.fetchViewDescription( sessionID, schemaName, viewName );
      }

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
                                 bool cacheSizeInMB ) const
      {
        return m_facade.fetchRows( sessionID, qd, pOutputBuffer, cacheSize, cacheSizeInMB );
      }

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
                                 bool cacheSizeInMB ) const
      {
        return m_facade.fetchRows( sessionID, qd, outputTypes, cacheSize, cacheSizeInMB );
      }

      /// Fetch all rows from a new query.
      /// This method does NOT keep any server-side open cursors (PUSH model).
      /// If given, the row buffer must be used by the iterator returning
      /// the results (and its specification must be shipped over the network
      /// to be used by the server-side IQuery::defineOutput call).
      IRowIteratorPtr fetchAllRows( Token sessionID,
                                    const QueryDefinition& qd,
                                    AttributeList* pOutputBuffer ) const
      {
        return m_facade.fetchAllRows( sessionID, qd, pOutputBuffer );
      }

      /// Fetch all rows from a new query.
      /// This method does NOT keep any server-side open cursors (PUSH model).
      /// The output type map must be shipped over the network to be used
      /// by the server-side IQuery::defineOutputTypes call).
      IRowIteratorPtr fetchAllRows( Token sessionID,
                                    const QueryDefinition& qd,
                                    const std::map< std::string, std::string > outputTypes ) const
      {
        return m_facade.fetchAllRows( sessionID, qd, outputTypes );
      }

      /// Execute a stored procedure with input parameters.
      void callProcedure( Token sessionID,
                          const std::string& schemaName,
                          const std::string& procedureName,
                          const coral::AttributeList& inputArguments ) const
      {
        return m_facade.callProcedure( sessionID, schemaName, procedureName, inputArguments );
      }

      // --- FUNCTIONALITIES NEEDED ONLY IN READ-WRITE MODE ---

      /// Delete rows from a table.
      /// Eventually pass an AttributeList.
      int deleteTableRows( Token sessionID,
                           const std::string& schemaName,
                           const std::string& tableName,
                           const std::string& whereClause,
                           const std::string& whereData ) const
      {
        return m_facade.deleteTableRows( sessionID, schemaName, tableName, whereClause, whereData );
      }

      /// Prepare a row buffer from the table metadata.
      /// Eventually retrieve an AttributeList.
      /// NB Eventually this will be replaced by other metadata methods.
      const std::string formatRowBufferAsString( Token sessionID,
                                                 const std::string& schemaName,
                                                 const std::string& tableName ) const
      {
        return m_facade.formatRowBufferAsString( sessionID, schemaName, tableName );
      }

      /// Insert one row.
      /// Eventually pass an AttributeList reference.
      void insertRowAsString( Token sessionID,
                              const std::string& schemaName,
                              const std::string& tableName,
                              const std::string& rowBufferAS ) const
      {
        return m_facade.insertRowAsString( sessionID, schemaName, tableName, rowBufferAS );
      }

      /// Get the bulkOperationID for a new bulk insertion.
      /// Eventually pass an AttributeList reference.
      /// NB Is the rowBuffer here really needed or I got it wrong?
      Token bulkInsertAsString( Token sessionID,
                                const std::string& schemaName,
                                const std::string& tableName,
                                const std::string& rowBufferAS,
                                int rowCacheSizeDb ) const
      {
        return m_facade.bulkInsertAsString( sessionID, schemaName, tableName, rowBufferAS, rowCacheSizeDb );
      }

      /// Release a bulk operation
      void releaseBulkOp( Token bulkOpID ) const
      {
        return m_facade.releaseBulkOp( bulkOpID );
      }

      /// Insert several rows in bulk.
      void processRows( Token bulkOpID,
                        const std::vector<AttributeList>& rowsAS ) const
      {
        return m_facade.processRows( bulkOpID, rowsAS );
      }

      /// Flush a bulk operation.
      void flush( Token bulkOpID ) const
      {
        return m_facade.flush( bulkOpID );
      }

    private:

      /// The CORAL server facade.
      CoralServerFacade m_facade;

    };

  }

}
#endif // CORALSERVER_CORALSERVERFACADESERVICE_H
