#ifndef CORAL_CORALSTUBS_DUMMYFACADE_H
#define CORAL_CORALSTUBS_DUMMYFACADE_H

//C++ includes
#include <string>

//Coral includes
#include "RelationalAccess/AccessMode.h"

#include "CoralServerBase/ICoralFacade.h"

namespace coral {

  namespace CoralStubs {

    /** @class DummyFacade
     *
     *  An inheritance of the ICoralFacade used for testing and debug mode only
     *
     *  This is realy only a tests class
     *
     *  @author Alexander Kalkhof
     *  @date   2009-01-29
     */

    class DummyFacade : public ICoralFacade {
    public:

      // Constructor
      DummyFacade();

      // Virtual destructor.
      virtual ~DummyFacade();

      void setCertificateData( const coral::ICertificateData* cert );

      Token connect( const std::string& dbUrl, const coral::AccessMode mode, bool& fromProxy ) const;

      void releaseSession( coral::Token sessionID ) const;

      void startTransaction( coral::Token sessionID, bool readOnly = false ) const;

      void commitTransaction( coral::Token sessionID ) const;

      void rollbackTransaction( coral::Token sessionID ) const;

      const std::vector<std::string> fetchSessionProperties( Token sessionID ) const;

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

    };

  }

}

#endif
