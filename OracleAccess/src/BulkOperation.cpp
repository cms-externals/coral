#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include "oci.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralCommon/SimpleTimer.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/SchemaException.h"

#include "BulkOperation.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "PolymorphicVector.h"
#include "SessionProperties.h"
#include "StatementStatistics.h"

coral::OracleAccess::BulkOperation::BulkOperation( boost::shared_ptr<const SessionProperties> properties,
                                                   const std::string& schemaName,
                                                   const coral::AttributeList& inputBuffer,
                                                   int cacheSize,
                                                   const std::string& statement ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_inputBuffer( inputBuffer ),
  m_rowsInCache( ( ( cacheSize > 65534 ) ? 65534 : cacheSize ) ),
  m_rowsUsed( 0 ),
  m_ociStmtHandle( 0 ),
  m_dataCache( 0 ),
  m_statementStatistics( 0 )
{
  // Prepare the statement
  sword status = OCIStmtPrepare2( m_sessionProperties->ociSvcCtxHandle(),
                                  &m_ociStmtHandle,
                                  m_sessionProperties->ociErrorHandle(),
                                  reinterpret_cast< CONST text* >( statement.c_str() ),
                                  ::strlen( statement.c_str() ),
                                  0,0,
                                  OCI_NTV_SYNTAX, OCI_DEFAULT );
  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status,"preparing the statement \"" + statement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    this->reset();
    throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                      "Could not prepare a new statement handle",
                                      "IBulkOperation" );
  }

  coral::MessageStream log( m_sessionProperties->domainServiceName() );
  log << coral::Debug << "Prepared for bulk operation statement \"" << statement << "\"" << coral::MessageStream::endmsg;


  // Prepare the data structures
  for ( coral::AttributeList::const_iterator iAttribute = m_inputBuffer.begin();
        iAttribute != m_inputBuffer.end(); ++iAttribute ) {
    const std::type_info& attributeType = iAttribute->specification().type();
    if ( attributeType == typeid(float) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<float>( m_rowsInCache, m_sessionProperties->serverVersion() ) );
    }
    else if ( attributeType == typeid(double) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<double>( m_rowsInCache, m_sessionProperties->serverVersion() ) );
    }
    else if ( attributeType == typeid(long double) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<long double>( m_rowsInCache, m_sessionProperties->serverVersion() ) );
    }
    else if ( attributeType == typeid(int) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<int>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(unsigned int) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<unsigned int>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(short) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<short>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(unsigned short) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<unsigned short>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(long) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<long>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(unsigned long) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<unsigned long>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(long long) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<long long>( m_rowsInCache, m_sessionProperties->ociErrorHandle() ) );
    }
    else if ( attributeType == typeid(unsigned long long) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<unsigned long long>( m_rowsInCache, m_sessionProperties->ociErrorHandle() ) );
    }
    else if ( attributeType == typeid(bool) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<bool>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(char) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<char>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(unsigned char) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<unsigned char>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(std::string) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<std::string>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(coral::Blob) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<coral::Blob>( m_rowsInCache,
                                                                                       m_sessionProperties->ociEnvHandle(),
                                                                                       m_sessionProperties->ociSvcCtxHandle(),
                                                                                       m_sessionProperties->ociErrorHandle() ) );
    }
    else if ( attributeType == typeid(coral::Date) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<coral::Date>( m_rowsInCache ) );
    }
    else if ( attributeType == typeid(coral::TimeStamp) ) {
      m_dataCache.push_back( new coral::OracleAccess::PolymorphicTVector<coral::TimeStamp>( m_rowsInCache,
                                                                                            m_sessionProperties->ociEnvHandle(),
                                                                                            m_sessionProperties->ociErrorHandle() ) );
    }
    else {
      log << coral::Error << "Unsupported type : \"" + iAttribute->specification().typeName() + "\"" << coral::MessageStream::endmsg;
      this->reset();
      throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                        "Could not prepare a new bulk operation",
                                        "IBulkOperation" );
    }
  }

  if ( m_sessionProperties->monitoringService() )
    m_statementStatistics = new coral::OracleAccess::StatementStatistics( statement );
}


coral::OracleAccess::BulkOperation::~BulkOperation()
{
  this->reset();
}


void
coral::OracleAccess::BulkOperation::processNextIteration()
{
  if ( coral::MessageStream::msgVerbosity() == coral::Verbose )
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    std::stringstream msg;
    m_inputBuffer.toOutputStream( msg );
    log << coral::Verbose << "Process next iteration: " << msg.str() << coral::MessageStream::endmsg;
  }
  // Copy the elements to the structures.
  unsigned int iVariable = 0;
  for ( coral::AttributeList::const_iterator iAttribute = m_inputBuffer.begin();
        iAttribute != m_inputBuffer.end(); ++iAttribute, ++iVariable ) {
    m_dataCache[iVariable]->push_back( iAttribute->addressOfData(),
                                       ( iAttribute->isNull() ? -1 : 0 ) );
  }
  ++m_rowsUsed;

  // If necessary flush the cache.
  if ( m_rowsUsed >= m_rowsInCache )
    this->flush();
}


static sb4
BindCallBackForStrings( dvoid* octxp,
                        OCIBind* /* bindp */,
                        ub4 iter,
                        ub4 /* index */,
                        dvoid** bufpp,
                        ub4* alenpp,
                        ub1* piecep,
                        dvoid** indpp )
{
  coral::OracleAccess::PolymorphicTVector<std::string>* dataVector =
    dynamic_cast< coral::OracleAccess::PolymorphicTVector<std::string>* >( static_cast<coral::OracleAccess::PolymorphicVector*>(octxp) );
  const std::string& data = dataVector->data( iter );
  *bufpp  = const_cast<char*>( data.c_str() );
  *alenpp = data.size() + 1;
  *piecep = OCI_ONE_PIECE;
  *indpp = dataVector->indicatorData( iter );
  return OCI_CONTINUE;
}


void
coral::OracleAccess::BulkOperation::flush()
{
  if ( m_rowsUsed == 0 ) return;
  coral::MessageStream log( m_sessionProperties->domainServiceName() );
  // Bind the data.
  ub4 position = 1;
  for ( std::vector< coral::OracleAccess::PolymorphicVector* >::iterator iVariable = m_dataCache.begin();
        iVariable != m_dataCache.end(); ++iVariable, ++position ) {
    OCIBind* ociBindHandle = 0;
    ub4 mode = OCI_DEFAULT;
    if ( (*iVariable)->dty() == SQLT_STR ) mode = OCI_DATA_AT_EXEC;
    sword status = OCIBindByPos( m_ociStmtHandle,
                                 &ociBindHandle,
                                 m_sessionProperties->ociErrorHandle(),
                                 position,
                                 const_cast<void*>( (*iVariable)->startingAddress() ),
                                 (*iVariable)->sizeOfObject(),
                                 (*iVariable)->dty(),
                                 (*iVariable)->indicatorArray(),
                                 (*iVariable)->lengthArray(),
                                 0, 0, 0,
                                 mode );
    if ( status != OCI_SUCCESS ) {
      coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "binding by position the input variables" );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      this->reset();
      throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                        "Could not bind a variable",
                                        "IBulkOperation::flush" );
    }

    if ( mode == OCI_DATA_AT_EXEC ) { // strings are special...
      status = OCIBindDynamic( ociBindHandle,
                               m_sessionProperties->ociErrorHandle(),
                               static_cast<void*>(*iVariable),
                               &BindCallBackForStrings,
                               0, 0 );
      if ( status != OCI_SUCCESS ) {
        coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );;
        errorHandler.handleCase( status, "binding the array structure of input variables" );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        this->reset();
        throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                          "Could not bind dynamically a variable",
                                          "IBulkOperation::flush" );
      }
    }
    else {
      status = OCIBindArrayOfStruct( ociBindHandle,
                                     m_sessionProperties->ociErrorHandle(),
                                     (*iVariable)->dataValueSkip(),
                                     sizeof(sb2),
                                     (*iVariable)->dataLengthSkip(),
                                     0 );
      if ( status != OCI_SUCCESS ) {
        coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
        errorHandler.handleCase( status, "binding the array structure of input variables" );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        this->reset();
        throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                          "Could not bind the array structure of a variable",
                                          "IBulkOperation::flush" );
      }
    }
  }


  // execute
  log << coral::Debug << "Executing a bulk operation (flushing the cache)" << coral::MessageStream::endmsg;

  coral::SimpleTimer timer;

  if ( m_statementStatistics )
    timer.start();

  ub4 iters = m_rowsUsed;
  sword status = OCIStmtExecute( m_sessionProperties->ociSvcCtxHandle(),
                                 m_ociStmtHandle,
                                 m_sessionProperties->ociErrorHandle(),
                                 iters, 0, 0, 0, OCI_DEFAULT );
  if ( m_statementStatistics )
    timer.stop();

  if ( status != OCI_SUCCESS ) {
    coral::OracleAccess::OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "executing the bulk operation statement" );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    this->reset();
    throw coral::DataEditorException( m_sessionProperties->domainServiceName(),
                                      "Could not execute a bulk operation",
                                      "IBulkOperation::flush" );
  }

  if ( m_statementStatistics )
  {
    double idleTimeSeconds = timer.total() * 1e-6;
    m_sessionProperties->monitoringService()->record( "oracle://" + m_sessionProperties->connectionString() + "/" + m_schemaName,
                                                      coral::monitor::Statement,
                                                      coral::monitor::Time,
                                                      m_statementStatistics->sqlStatement,
                                                      idleTimeSeconds );
  }

  // Clear the cache
  for ( std::vector< coral::OracleAccess::PolymorphicVector* >::iterator iVariable = m_dataCache.begin();
        iVariable != m_dataCache.end(); ++iVariable ) (*iVariable)->clear();
  m_rowsUsed = 0;
}


void
coral::OracleAccess::BulkOperation::reset()
{
  for ( std::vector< coral::OracleAccess::PolymorphicVector* >::iterator iVariable = m_dataCache.begin();
        iVariable != m_dataCache.end(); ++iVariable ) delete *iVariable;
  m_dataCache.clear();
  m_rowsUsed = 0;

  if ( m_ociStmtHandle ) {
    m_sessionProperties->releaseOCIStmt( m_ociStmtHandle );
    m_ociStmtHandle = 0;
  }
  
  if ( m_statementStatistics ) {
    delete m_statementStatistics;
    m_statementStatistics = 0;
  }
}
