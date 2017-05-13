#ifndef CORAL_CORALSTUBS_CORALFACADE_H
#define CORAL_CORALSTUBS_CORALFACADE_H

//C++ includes
#include <string>

//Coral includes
#include "RelationalAccess/AccessMode.h"

#include "CoralServerBase/ICoralFacade.h"
#include "CoralServerBase/wintypes.h"

namespace coral {

  // Forward declarations
  class IRequestHandler;

  namespace CoralStubs {

    /** @class ClientStub
     *
     *  Implementation of the ICoralFacade used for the client side.
     *
     *  @author Alexander Kalkhof
     *  @date   2009-01-28
     */

    class ClientStub : public ICoralFacade
    {
    public:

      // Destructor.
      virtual ~ClientStub();

      // Constructor from an IRequestHandler.
      ClientStub( IRequestHandler& requestHandler );

      void setCertificateData( const coral::ICertificateData* cert );

      //connect to a database backend
      Token connect( const std::string& dbUrl, const coral::AccessMode mode, bool& fromProxy ) const;

      //release the session and release the database connection
      void releaseSession( coral::Token sessionID ) const;

      const std::vector<std::string> fetchSessionProperties( Token sessionID ) const;

      void startTransaction( coral::Token sessionID, bool readOnly = false ) const;

      void commitTransaction( coral::Token sessionID ) const;

      void rollbackTransaction( coral::Token sessionID ) const;

      const std::set<std::string> listTables( Token sessionID, const std::string& schemaName ) const;

      bool existsTable( Token sessionID, const std::string& schemaName, const std::string& tableName ) const;

      const TableDescription fetchTableDescription( Token sessionID, const std::string& schemaName, const std::string& tableName ) const;

      const std::set<std::string> listViews( Token sessionID, const std::string& schemaName ) const;

      bool existsView( Token sessionID, const std::string& schemaName, const std::string& viewName ) const;

      const std::pair<TableDescription,std::string> fetchViewDescription( Token sessionID, const std::string& schemaName, const std::string& viewName ) const;

      IRowIteratorPtr fetchRows( Token sessionID, const QueryDefinition& qd, AttributeList* pRowBuffer, size_t cacheSize, bool cacheSizeInMB ) const;

      IRowIteratorPtr fetchRows( Token sessionID, const QueryDefinition& qd, const std::map< std::string, std::string > outputTypes, size_t cacheSize, bool cacheSizeInMB ) const;

      IRowIteratorPtr fetchAllRows( Token sessionID, const QueryDefinition& qd, AttributeList* rowBuffer ) const;

      IRowIteratorPtr fetchAllRows( Token sessionID, const QueryDefinition& qd, const std::map< std::string, std::string > outputTypes ) const;

      void callProcedure( Token sessionID, const std::string& schemaName, const std::string& procedureName, const coral::AttributeList& inputArguments ) const;

      int deleteTableRows( Token sessionID, const std::string& schemaName, const std::string& tableName, const std::string& whereClause, const std::string& whereData ) const;

      const std::string formatRowBufferAsString( Token sessionID, const std::string& schemaName, const std::string& tableName ) const;

      void insertRowAsString( Token sessionID, const std::string& schemaName, const std::string& tableName, const std::string& rowBufferAS ) const;

      Token bulkInsertAsString( Token sessionID, const std::string& schemaName, const std::string& tableName, const std::string& rowBufferAS, int rowCacheSizeDb ) const;

      void releaseBulkOp( Token bulkOpID ) const;

      void processRows( Token bulkOpID, const std::vector<coral::AttributeList>& rowsAS ) const;

      void flush( Token bulkOpID ) const;

    private:

      // Reference to an IRequestHandler instance
      IRequestHandler& m_requestHandler;

    };

  }

}

#endif
