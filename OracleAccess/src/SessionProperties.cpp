#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <iostream>
#include "oci.h"

#include "CoralBase/boost_datetime_headers.h"
#include "CoralCommon/Utilities.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/SessionException.h"

#include "ConnectionProperties.h"
#include "DomainProperties.h"
#include "Session.h"
#include "SessionProperties.h"

coral::OracleAccess::SessionProperties::SessionProperties( boost::shared_ptr<ConnectionProperties> connectionProperties,
                                                           const std::string& schemaName,
                                                           Session& session,
                                                           bool readOnly ) :
  m_connectionProperties( connectionProperties ),
  m_domainServiceName( connectionProperties->domainServiceName() ),
  m_session( &session ),
  m_ociSvcCtxHandle( 0 ),
  m_monitoringService( 0 ),
  m_schemaName( schemaName ),
  m_isReadOnly( readOnly ),
  m_mutex(),
  m_schemaMutex(),
  //m_transactionMutex(),
  m_selectAnyTable( true )
{
  //std::cout << "Create SessionProperties " << this << std::endl; // debug bug #73334
}

coral::OracleAccess::SessionProperties::~SessionProperties()
{
  //std::cout << "Delete SessionProperties " << this << std::endl; // debug bug #73334
}


void
coral::OracleAccess::SessionProperties::invalidateSession()
{
  //std::cout << "SessionProperties " << this << ": invalidateSession" << std::endl; // debug bug #80174
  if ( getenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S" ) )
  {
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::invalidateSession() @"
              << time1 << "... (test bug #80065)" << std::endl;
  }
  boost::mutex::scoped_lock lockS(m_schemaMutex); // fixes bug #80065 & #80174!
  //boost::mutex::scoped_lock lockT(m_transactionMutex); // fixes bug #80174?
  m_session = NULL;
  if ( getenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S" ) )
  {
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::invalidateSession() @"
              << time2 << " OK (test bug #80065)" << std::endl;
  }
}


bool
coral::OracleAccess::SessionProperties::isTransactionActive() const
{
  //boost::mutex::scoped_lock lock(m_transactionMutex); // fix bug #80174?
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::isTransactionActive",
                                   domainServiceName() );
  // Test multi-threaded bug #80174
  // Without a mutex, this crashes...
  if ( getenv( "CORAL_ORA_TEST_BUG80174_SLEEP5S" ) )
  {
    coral::ITransaction& transaction = m_session->transaction();
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::isTransactionActive() @"
              << time1 << " sleep 5s (test bug #80174)" << std::endl;
    coral::sleepSeconds(5);
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::isTransactionActive() @"
              << time2 << " slept 5s (test bug #80174)" << std::endl;
    return transaction.isActive();
  }
  // Test multi-threaded bug #80174
  //if ( !m_session->isUserSessionActive() ) // fix bug #80098?
  //  throw coral::SessionException( "Session is no longer valid",
  //                                 "SessionProperties::isTransactionActive",
  //                                 domainServiceName() );
  //std::cout << "SessionProperties:::isTransactionActive: Session is " << m_session << std::endl; // debug bug #80098
  return m_session->transaction().isActive();
}


bool
coral::OracleAccess::SessionProperties::isTransactionReadOnly() const
{
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::isTransactionReadOnly",
                                   domainServiceName() );
  return m_session->transaction().isReadOnly();
}


bool
coral::OracleAccess::SessionProperties::selectAnyTable() const
{
  return m_selectAnyTable;
}


void
coral::OracleAccess::SessionProperties::cannotSelectAnyTable() const
{
  m_selectAnyTable = false;
}


bool
coral::OracleAccess::SessionProperties::existsTable( const std::string& schemaName,
                                                     const std::string& tableName ) const
{
  boost::mutex::scoped_lock lock(m_schemaMutex); // fix bug #80065
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::session",
                                   domainServiceName() );
  return m_session->schema( schemaName ).existsTable( tableName );
}

bool
coral::OracleAccess::SessionProperties::existsView( const std::string& schemaName,
                                                    const std::string& viewName ) const
{
  boost::mutex::scoped_lock lock(m_schemaMutex); // fix bug #80065
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::session",
                                   domainServiceName() );
  return m_session->schema( schemaName ).existsView( viewName );
}


const std::vector<std::string>
coral::OracleAccess::SessionProperties::describeTable( const std::string& schemaName,
                                                       const std::string& tableName ) const
{
  boost::mutex::scoped_lock lock(m_schemaMutex); // fix bug #80065
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::session",
                                   domainServiceName() );
  // Test multi-threaded bug #80178 or bug #81112: START
  // Without a mutex, this crashes...
  if ( getenv( "CORAL_ORA_TEST_BUG80178_SLEEP5S" ) || 
       getenv( "CORAL_ORA_TEST_BUG81112_SLEEP5S" ) )
  {
    std::vector<std::string> colNames;
    ISchema& schema = m_session->schema( schemaName );
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::describeTable() @"
              << time1 << " sleep 5s (test bug #80178 and bug #81112)" 
              << std::endl;
    coral::sleepSeconds(5);
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::describeTable() @"
              << time2 << " slept 5s (test bug #80178 and bug #81112)" 
              << std::endl;
    const ITableDescription& desc = schema.tableHandle( tableName ).description();
    for ( int iCol = 0; iCol < desc.numberOfColumns(); ++iCol )
      colNames.push_back( desc.columnDescription(iCol).name() );
    return colNames;
  }
  // Test multi-threaded bug #80178 and bug #81112: END
  // Test multi-threaded bug #80065 (same as single-threaded bug #73834): START
  if ( getenv( "CORAL_ORA_TEST_BUG80065_SLEEP5S" ) )
  {
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::describeTable() @"
              << time1 << " sleep 5s (test bug #80065)" << std::endl;
    coral::sleepSeconds(5);
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::describeTable() @"
              << time2 << " slept 5s (test bug #80065)" << std::endl;
  }
  // Test multi-threaded bug #80065 (same as single-threaded bug #73834): END
  std::vector<std::string> colNames;
  const ITableDescription& desc = m_session->schema( schemaName ).tableHandle( tableName ).description();
  for ( int iCol = 0; iCol < desc.numberOfColumns(); ++iCol )
    colNames.push_back( desc.columnDescription(iCol).name() );
  return colNames;
}


const std::vector<std::string>
coral::OracleAccess::SessionProperties::describeView( const std::string& schemaName,
                                                      const std::string& viewName ) const
{
  boost::mutex::scoped_lock lock(m_schemaMutex); // fix bug #80065
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::session",
                                   domainServiceName() );
  std::vector<std::string> colNames;
  const IView& view = m_session->schema( schemaName ).viewHandle( viewName );
  for ( int iCol = 0; iCol < view.numberOfColumns(); ++iCol )
    colNames.push_back( view.column(iCol).name() );
  return colNames;
}


coral::IView&
coral::OracleAccess::SessionProperties::viewHandle( const std::string& schemaName,
                                                    const std::string& viewName ) const
{
  boost::mutex::scoped_lock lock(m_schemaMutex); // fix bug #80065
  if ( !m_session )
    throw coral::SessionException( "Session is no longer valid",
                                   "SessionProperties::session",
                                   domainServiceName() );
  return m_session->schema( schemaName ).viewHandle( viewName );
}


std::string
coral::OracleAccess::SessionProperties::cppTypeForSqlType( const std::string& sqlType ) const
{
  // NB do not lock the schema mutex (deadlock due to recursive calls)
  // Test multi-threaded bug #80097: START
  if ( getenv( "CORAL_ORA_TEST_BUG80097_SLEEP5S" ) )
  {
    ITypeConverter& typeConverter = m_connectionProperties->typeConverter();
    std::string time1 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::cppTypeForSqlType() @"
              << time1 << " sleep 5s (test bug #80097)" << std::endl;
    coral::sleepSeconds(5);
    std::string time2 = boost::posix_time::to_simple_string( boost::posix_time::microsec_clock::local_time() ).substr(12);
    std::cout << "__SessionProperties::cppTypeForSqlType() @"
              << time2 << " slept 5s (test bug #80097)" << std::endl;
    return typeConverter.cppTypeForSqlType( sqlType );
  }
  // Test multi-threaded bug #80097: END
  return m_connectionProperties->typeConverter().cppTypeForSqlType( sqlType );
}


std::string
coral::OracleAccess::SessionProperties::sqlTypeForCppType( const std::string& cppType ) const
{
  // NB do not lock the schema mutex (deadlock due to recursive calls)
  return m_connectionProperties->typeConverter().sqlTypeForCppType( cppType );
}


void 
coral::OracleAccess::SessionProperties::releaseOCIStmt( OCIStmt* ociStmtHandle ) const
{
  boost::mutex::scoped_lock lock(m_mutex);
  // Workaround for bug #83601 in Oracle 11g client (warning: memory leak?)
  if ( m_ociSvcCtxHandle ) 
  {
    //std::cout << "Delete OCIStmt... " << std::endl; // bug #83601
    OCIStmtRelease ( ociStmtHandle, ociErrorHandle(), 0, 0, OCI_DEFAULT );
    //std::cout << "OCIStmt deleted " << std::endl; // bug #83601
  }
  //else
  {
    //std::cout << "OCIStmt NOT deleted " << std::endl; // bug #83601
  }        
}
