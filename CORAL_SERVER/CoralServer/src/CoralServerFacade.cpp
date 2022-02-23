// $Id: CoralServerFacade.cpp,v 1.58.2.1 2010/12/20 09:10:10 avalassi Exp $

// Include files
#include <fstream>
#include <memory>
#include <iostream>
#include <sstream>
#include "CoralKernel/Context.h"
#include "CoralServer/CoralServerFacade.h"
#include "CoralServerBase/InternalErrorException.h"
#include "CoralServerBase/NotImplemented.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProperties.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/IView.h"

// Local include files
#include "CursorIterator.h"
#include "CoralMonitor/StopTimer.h"
#include "ObjectStoreMgr.h"
//#include "RowVectorIterator.h"
//#include "QueryAndCursor.h"
#include "QueryMgr.h"
#include "SessionAndFlag.h"

// Logger
#define LOGGER_NAME "CoralServer::CoralServerFacade"
#include "logger.h"

// Namespace
using namespace coral::CoralServer;

//-----------------------------------------------------------------------------

CoralServerFacade::CoralServerFacade( ITokenPool* tokenPool )
  : m_pConnSvc()
  , m_storeMgr( new ObjectStoreMgr( tokenPool ) )
  , m_certificateData( 0 )
{
  logger << Debug << "Create CoralServerFacade" << endlog;
  // Use the second connection service if it exists (local facade mode)
  // Use the default connection service otherwise
  // [do not attempt to load it - it must already exist if you are here!]
  m_pConnSvc = Context::instance().query<ICertificateConnectionService>( "CORAL/Services/ConnectionService2" );
  // FIXME - remove this line? always use ConnectionService2?
  if ( !m_pConnSvc.isValid() )
    m_pConnSvc = Context::instance().query<ICertificateConnectionService>( "CORAL/Services/ConnectionService" );
  if ( !m_pConnSvc.isValid() )
    throw Exception( "Could not retrieve the connection service",
                     "CoralServerFacade::CoralServerFacade",
                     "coral::CoralServer" );
  // Disable conn sharing unless CORALSERVER_ENABLECONNECTIONSHARING is set
  if ( getenv( "CORALSERVER_ENABLECONNECTIONSHARING" ) )
  {
    logger << Debug << "Enable connection sharing (CORALSERVER_ENABLECONNECTIONSHARING is set)" << endlog;
    m_pConnSvc->configuration().enableConnectionSharing();
  }
  else
  {
    logger << Debug << "Disable connection sharing (CORALSERVER_ENABLECONNECTIONSHARING is NOT set)" << endlog;
    m_pConnSvc->configuration().disableConnectionSharing();
  }

}

//-----------------------------------------------------------------------------

CoralServerFacade::~CoralServerFacade()
{
  logger << Debug << "Delete CoralServerFacade..." << endlog;
  m_pConnSvc = IHandle<ICertificateConnectionService>();
  if ( m_storeMgr ) delete m_storeMgr;
  m_storeMgr = 0;
  logger << Debug << "Deleted CoralServerFacade... DONE" << endlog;

  // print out timer if enabled
  //printTimers();
}

//-----------------------------------------------------------------------------

coral::Token CoralServerFacade::connect( const std::string& dbUrl,
                                         const coral::AccessMode mode,
                                         bool& fromProxy ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::connnect()");
  logger << Debug << "CoralServerFacade::connect( "
         << dbUrl << ", " << mode << ", " << fromProxy << " )" << endlog;
  //fromProxy = true; // HACK to test proxy behaviour
  ICertificateConnectionService& connSvc = const_cast<ICertificateConnectionService&>( *m_pConnSvc );
  // NB1: do not specify any role (coral_default_role would be wrong!)
  // NB2: pass certificateData even if == 0
  ISessionProxy* session = connSvc.connect( dbUrl, mode, m_certificateData );
  logger << Debug << "Got a new session" << endlog;
  if ( fromProxy ) session->transaction().start( true );  // RO PROXY
  SessionAndFlag* saf = new SessionAndFlag( session, fromProxy );
  return m_storeMgr->registerObject( saf, 0 );
}

//-----------------------------------------------------------------------------

void CoralServerFacade::releaseSession( coral::Token sessionID ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::releaseSession()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  if ( saf->fromProxy() ) saf->session()->transaction().commit();  // RO PROXY
  m_storeMgr->releaseObject( sessionID );
}

//-----------------------------------------------------------------------------

const std::vector<std::string>
CoralServerFacade::fetchSessionProperties( coral::Token sessionID ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchSessionProperties()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  std::vector<std::string> properties;
  properties.push_back( session->properties().flavorName() );
  properties.push_back( session->properties().serverVersion() );
  properties.push_back( session->nominalSchema().schemaName() );
  return properties;
}

//-----------------------------------------------------------------------------

void CoralServerFacade::startTransaction( coral::Token sessionID,
                                          bool readOnly ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::startTransaction()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  if ( saf->fromProxy() )
    throw InternalErrorException( "PANIC! Proxy did not drop startTransaction message",
                                  "CoralServerFacade::startTransaction",
                                  "coral::CoralServer" );
  ISessionProxy* session = saf->session();
  session->transaction().start( readOnly );
}

//-----------------------------------------------------------------------------

void CoralServerFacade::commitTransaction( coral::Token sessionID ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::commitTransaction()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  if ( saf->fromProxy() )
    throw InternalErrorException( "PANIC! Proxy did not drop commitTransaction message",
                                  "CoralServerFacade::commitTransaction",
                                  "coral::CoralServer" );
  ISessionProxy* session = saf->session();
  session->transaction().commit();
}

//-----------------------------------------------------------------------------

void CoralServerFacade::rollbackTransaction( coral::Token sessionID ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::rollbackTransaction()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  if ( saf->fromProxy() )
    throw InternalErrorException( "PANIC! Proxy did not drop rollbackTransaction message",
                                  "CoralServerFacade::rollbackTransaction",
                                  "coral::CoralServer" );
  ISessionProxy* session = saf->session();
  session->transaction().rollback();
}

//-----------------------------------------------------------------------------

const std::set<std::string>
CoralServerFacade::listTables( Token sessionID,
                               const std::string& schemaName ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::listTables()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  return session->schema( schemaName ).listTables();
}

//-----------------------------------------------------------------------------

bool CoralServerFacade::existsTable( Token sessionID,
                                     const std::string& schemaName,
                                     const std::string& tableName ) const
{
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  return session->schema( schemaName ).existsTable( tableName );
}

//-----------------------------------------------------------------------------

const coral::TableDescription
CoralServerFacade::fetchTableDescription( Token sessionID,
                                          const std::string& schemaName,
                                          const std::string& tableName ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchTableDescription()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  return session->schema( schemaName ).tableHandle( tableName ).description();
}

//-----------------------------------------------------------------------------

const std::set<std::string>
CoralServerFacade::listViews( Token sessionID,
                              const std::string& schemaName ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::listViews()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  return session->schema( schemaName ).listViews();
}

//-----------------------------------------------------------------------------

bool CoralServerFacade::existsView( Token sessionID,
                                    const std::string& schemaName,
                                    const std::string& viewName ) const
{
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  return session->schema( schemaName ).existsView( viewName );
}

//-----------------------------------------------------------------------------

const std::pair<coral::TableDescription,std::string>
CoralServerFacade::fetchViewDescription( Token sessionID,
                                         const std::string& schemaName,
                                         const std::string& viewName ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchViewDescription()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  ISchema& schema = session->schema( schemaName );
  const IView& view = schema.viewHandle( viewName );
  // Build a view 'description' (with only the data relevant to the IView API)
  TableDescription desc;
  desc.setName( view.name() );
  for ( int i=0; i<view.numberOfColumns(); i++ )
  {
    const IColumn& col = view.column( i );
    desc.insertColumn( col.name(),
                       col.type(),
                       col.size(),
                       col.isSizeFixed(),
                       col.tableSpaceName() );
  }
  std::string def = view.definition();
  return std::pair<TableDescription,std::string>( desc, def );
}

//-----------------------------------------------------------------------------

void CoralServerFacade::callProcedure( Token sessionID,
                                       const std::string& schemaName,
                                       const std::string& procedureName,
                                       const coral::AttributeList& inputArguments ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFace::callProcedure()");
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  // No special handling if schemaName == ""
  ISchema& schema = session->schema( schemaName );
  schema.callProcedure( procedureName, inputArguments );
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
CoralServerFacade::fetchRows( coral::Token sessionID,
                              const coral::QueryDefinition& qd,
                              coral::AttributeList* pOutputBuffer,
                              size_t /*cacheSize*/,
                              bool /*cacheSizeInMB*/ ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchRows() OutputBuffer");
  return fetchAllRows( sessionID, qd, pOutputBuffer );
  /*
  //logger << Always << "CoralServerFacade::fetchRows show row buffer..." << endlog;
  //if ( !pOutputBuffer ) logger << Always << "NULL" << endlog;
  //else logger << Always << *pOutputBuffer << endlog;
  //logger << Always << "CoralServerFacade::fetchRows show row buffer... DONE" << endlog;
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  IQuery* query = QueryMgr::newQuery( pOutputBuffer, *session, qd );
  ICursor& cursor = query->execute();
  QueryAndCursor* qac = new QueryAndCursor( query, cursor, pOutputBuffer );
  return m_storeMgr->registerObject( qac, sessionID );
  */
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
CoralServerFacade::fetchRows( coral::Token sessionID,
                              const coral::QueryDefinition& qd,
                              const std::map< std::string, std::string > outputTypes,
                              size_t /*cacheSize*/,
                              bool /*cacheSizeInMB*/ ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchRows() outputTypes");
  return fetchAllRows( sessionID, qd, outputTypes );
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
CoralServerFacade::fetchAllRows( coral::Token sessionID,
                                 const coral::QueryDefinition& qd,
                                 coral::AttributeList* pOutputBuffer ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchAllRows() outputBuffer");
  //logger << Always << "CoralServerFacade::fetchAllRows show row buffer..." << endlog;
  //if ( !pOutputBuffer ) logger << Always << "NULL" << endlog;
  //else logger << Always << *pOutputBuffer << endlog;
  //logger << Always << "CoralServerFacade::fetchAllRows show row buffer... DONE" << endlog;
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  IQuery* query = QueryMgr::newQuery( pOutputBuffer, *session, qd );
  return IRowIteratorPtr( new CursorIterator( *query ) );
  /*
  std::unique_ptr<IQuery> query( QueryMgr::newQuery( pOutputBuffer, *session, qd ) );
  ICursor& cursor = query->execute();
  std::vector<AttributeList> rows;
  while ( cursor.next() ) rows.push_back( cursor.currentRow() );
  static int csdebug = -1;
  if ( csdebug == -1 ) csdebug = ( getenv ( "CORALSERVER_DEBUG" ) ? 1 : 0 );
  if ( csdebug > 0 )
  {
    if ( pOutputBuffer )
      std::cout << "Fetched all rows from database cursor (with output buffer): #rows=" << rows.size() << std::endl;
    //std::cout << "Fetched all rows from database cursor (with output buffer "
    //            << pOutputBuffer << "): #rows=" << rows.size() << std::endl;
    else
      std::cout << "Fetched all rows from database cursor (with null output buffer): #rows=" << rows.size() << std::endl;
  }
  return IRowIteratorPtr( new RowVectorIterator( pOutputBuffer, rows ) );
  */
}

//-----------------------------------------------------------------------------

coral::IRowIteratorPtr
CoralServerFacade::fetchAllRows( coral::Token sessionID,
                                 const coral::QueryDefinition& qd,
                                 const std::map< std::string, std::string > outputTypes ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::fetchAllRows() outputTypes");
  //logger << Always << "CoralServerFacade::fetchAllRows show row buffer..." << endlog;
  //if ( !pOutputBuffer ) logger << Always << "NULL" << endlog;
  //else logger << Always << *pOutputBuffer << endlog;
  //logger << Always << "CoralServerFacade::fetchAllRows show row buffer... DONE" << endlog;
  SessionAndFlag* saf = m_storeMgr->findObject<SessionAndFlag>( sessionID );
  ISessionProxy* session = saf->session();
  IQuery* query = QueryMgr::newQuery( outputTypes, *session, qd );
  return IRowIteratorPtr( new CursorIterator( *query ) );
  /*
  std::unique_ptr<IQuery> query( QueryMgr::newQuery( outputTypes, *session, qd ) );
  ICursor& cursor = query->execute();
  std::vector<AttributeList> rows;
  while ( cursor.next() ) rows.push_back( cursor.currentRow() );
  static int csdebug = -1;
  if ( csdebug == -1 ) csdebug = ( getenv ( "CORALSERVER_DEBUG" ) ? 1 : 0 );
  if ( csdebug > 0 )
    std::cout << "Fetched all rows from database cursor (with output types): #rows=" << rows.size() << std::endl;
  coral::AttributeList* pOutputBuffer = 0;
  return IRowIteratorPtr( new RowVectorIterator( pOutputBuffer, rows ) );
  */
}

//-----------------------------------------------------------------------------

int CoralServerFacade::deleteTableRows( coral::Token /*sessionID*/,
                                        const std::string& /*schemaName*/,
                                        const std::string& /*tableName*/,
                                        const std::string& /*whereClause*/,
                                        const std::string& /*whereData*/ ) const
{
  throw NotImplemented("CoralServerFacade::deleteTableRows");
}

//-----------------------------------------------------------------------------

const std::string
CoralServerFacade::formatRowBufferAsString( coral::Token /*sessionID*/,
                                            const std::string& /*schemaName*/,
                                            const std::string& /*tableName*/ ) const
{
  throw NotImplemented("CoralServerFacade::formatRowBufferAsString");
}

//-----------------------------------------------------------------------------

void CoralServerFacade::insertRowAsString( coral::Token /*sessionID*/,
                                           const std::string& /*schemaName*/,
                                           const std::string& /*tableName*/,
                                           const std::string& /*rowBufferAS*/ ) const
{
  throw NotImplemented("CoralServerFacade::insertRowAsString");
}

//-----------------------------------------------------------------------------

coral::Token
CoralServerFacade::bulkInsertAsString( coral::Token /*sessionID*/,
                                       const std::string& /*schemaName*/,
                                       const std::string& /*tableName*/,
                                       const std::string& /*rowBufferAS*/,
                                       int /*rowCacheSizeDb*/ ) const
{
  throw NotImplemented("CoralServerFacade::bulkInsertAsString");
}

//-----------------------------------------------------------------------------

void CoralServerFacade::releaseBulkOp( coral::Token bulkOpID ) const
{
  //ScopedTimer timer("CoralServer::CoralServerFacade::releaseBulkOp");
  m_storeMgr->releaseObject( bulkOpID );
}

//-----------------------------------------------------------------------------

void CoralServerFacade::processRows( coral::Token /*bulkOpID*/,
                                     const std::vector<coral::AttributeList>& /*rows*/ ) const
{
  throw NotImplemented("CoralServerFacade::processRows");
}

//-----------------------------------------------------------------------------

void CoralServerFacade::flush( coral::Token /*bulkOpID*/ ) const
{
  throw NotImplemented("CoralServerFacade::flush");
}

//-----------------------------------------------------------------------------
