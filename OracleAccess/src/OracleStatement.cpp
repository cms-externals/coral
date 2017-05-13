#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include "oci.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralCommon/MonitoringEventDescription.h"
#include "CoralCommon/SimpleTimer.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/SchemaException.h"

#include "BindVariableHolder.h"
#include "DomainProperties.h"
#include "OracleErrorHandler.h"
#include "OracleStatement.h"
#include "OutputVariableHolder.h"
#include "StatementStatistics.h"

coral::OracleAccess::OracleStatement::OracleStatement( boost::shared_ptr<const SessionProperties> properties,
                                                       const std::string& schemaName,
                                                       const std::string& sqlStatement ) :
  m_sessionProperties( properties ),
  m_schemaName( schemaName ),
  m_ociStmtHandle( 0 ),
  m_indicators(),
  m_outputData( 0 ),
  m_outputVariables(),
  m_statementStatistics( 0 ),
  m_sqlStatement( sqlStatement )
{
  //std::cout << "Create Statement " << this << ": " << m_sqlStatement << std::endl; // debug bug #73334
  coral::MessageStream log( m_sessionProperties->domainServiceName() );

  // Prepare the statement
  sword status;
  {
    //boost::mutex::scoped_lock lock( m_sessionProperties->connectionProperties().connectionMutex() );
    status = OCIStmtPrepare2( m_sessionProperties->ociSvcCtxHandle(),
                              &m_ociStmtHandle,
                              m_sessionProperties->ociErrorHandle(),
                              reinterpret_cast< CONST text* >( sqlStatement.c_str() ),
                              ::strlen( sqlStatement.c_str() ),
                              0, 0, OCI_NTV_SYNTAX, OCI_DEFAULT );
  }
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Preparing statement \"" + m_sqlStatement + "\"" );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                 "Could not prepare statement \"" + m_sqlStatement + "\"",
                                 "OracleStatement" );
  }
  //std::cout << "OCIStmt created " << this << std::endl; // bug #83601
  log << coral::Debug << "Prepared statement : \"" << m_sqlStatement << "\"" << coral::MessageStream::endmsg;

  if ( m_sessionProperties->monitoringService() )
    m_statementStatistics = new StatementStatistics( sqlStatement );
}


coral::OracleAccess::OracleStatement::~OracleStatement()
{
  //std::cout << "Delete Statement " << this << std::endl; // debug bug #73334
  this->reset();
}


void
coral::OracleAccess::OracleStatement::reset()
{
  // Get rid of the output variables
  for ( std::vector<IOutputVariableHolder*>::iterator iOutputVariable = m_outputVariables.begin(); iOutputVariable != m_outputVariables.end(); ++iOutputVariable ) 
    delete *iOutputVariable;
  m_outputVariables.clear();

  // Release the statement handle
  if ( m_ociStmtHandle )
    m_sessionProperties->releaseOCIStmt( m_ociStmtHandle );
  m_ociStmtHandle = 0;

  // Delete the statement statistics
  if ( m_statementStatistics )
  {
    delete m_statementStatistics;
  }
  m_statementStatistics = 0;
}


bool
coral::OracleAccess::OracleStatement::execute( const coral::AttributeList& bindData,
                                               sb4* errorCodePtr )
{
  if ( coral::MessageStream::msgVerbosity() == coral::Verbose )
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    std::stringstream msg;
    bindData.toOutputStream( msg );
    log << coral::Verbose << "Execute statement with bind data: " << msg.str() << coral::MessageStream::endmsg;
  }
  // Check whether this has been a select statement.
  ub2 statementType = 0;
  sword status = OCIAttrGet( m_ociStmtHandle, OCI_HTYPE_STMT,
                             &statementType,
                             0,
                             OCI_ATTR_STMT_TYPE,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving statement type for \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    if ( errorCodePtr != 0 ) *errorCodePtr = errorHandler.lastErrorCode();
    this->reset();
    return false;
  }
  sb2* indicators = 0;
  ub4 numberOfVariables = bindData.size();
  std::vector< IBindVariableHolder* > bindVariables;
  if ( numberOfVariables > 0 ) indicators = new sb2[numberOfVariables];
  // Bind the variables and the indicators
  int iVarInd = 0;
  for ( coral::AttributeList::const_iterator iAttribute = bindData.begin(); iAttribute != bindData.end(); ++iAttribute, ++iVarInd )
  {
    sb2* indicatorPointer = &indicators[iVarInd];
    *indicatorPointer = 0;
    void* attributeAddress = const_cast<void*>( iAttribute->addressOfData() );
    ub2 dty = 0;
    std::string attributeName = iAttribute->specification().name();
    const std::type_info& attributeType = iAttribute->specification().type();
    sb4 attributeSize = iAttribute->size();
    // Check the NULL-ness of the variable
    if ( iAttribute->isNull() ) *indicatorPointer = -1;
    // Check the type
    if ( attributeType == typeid(float) )
    {
      if ( m_sessionProperties->serverVersion() > 9 ) dty = SQLT_BFLOAT;
      else dty = SQLT_FLT;
    }
    else if ( attributeType == typeid(double) )
    {
      if ( m_sessionProperties->serverVersion() > 9 ) dty = SQLT_BDOUBLE;
      else dty = SQLT_FLT;
    }
    else if ( attributeType == typeid(long double) )
    {
      if ( m_sessionProperties->serverVersion() > 9 ) dty = SQLT_BDOUBLE;
      else dty = SQLT_FLT;
    }
    else if ( attributeType == typeid(int) ||
              attributeType == typeid(short) ||
              attributeType == typeid(long) ||
              attributeType == typeid(bool) )
    {
      dty = SQLT_INT;
    }
    else if ( attributeType == typeid(unsigned int) ||
              attributeType == typeid(unsigned short) ||
              attributeType == typeid(unsigned long) ||
              attributeType == typeid(unsigned char) )
    {
      dty = SQLT_UIN;
    }
    else if ( attributeType == typeid(char) )
    {
      dty = SQLT_AFC;
    }
    else if ( attributeType == typeid(std::string) )
    {
      dty = SQLT_STR;
      const char* sData = iAttribute->data<std::string>().c_str();
      attributeAddress = const_cast<char*>( sData );
      attributeSize = ::strlen( sData ) + 1;
    }
    else if ( attributeType == typeid(long long) )
    {
      bindVariables.push_back( new NumberVariableHolder( attributeAddress,
                                                         true,
                                                         m_sessionProperties,
                                                         attributeSize ) );
      const IBindVariableHolder& variableHolder = *( bindVariables.back() );
      if ( ! variableHolder.succeeded() )
      {
        delete [] indicators;
        for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
          delete *iVariable;
        this->reset();
        return false;
      }
      dty = SQLT_VNU;
      attributeAddress = variableHolder.bindAddress();
      attributeSize = variableHolder.size();
    }
    else if ( attributeType == typeid(unsigned long long) )
    {
      bindVariables.push_back( new NumberVariableHolder( attributeAddress,
                                                         false,
                                                         m_sessionProperties,
                                                         attributeSize ) );
      const IBindVariableHolder& variableHolder = *( bindVariables.back() );
      if ( ! variableHolder.succeeded() )
      {
        delete [] indicators;
        for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
          delete *iVariable;
        this->reset();
        return false;
      }
      dty = SQLT_VNU;
      attributeAddress = variableHolder.bindAddress();
      attributeSize = variableHolder.size();
    }
    else if ( attributeType == typeid(coral::Blob) )
    {
      bindVariables.push_back( new BlobVariableHolder( *( static_cast<coral::Blob*>( attributeAddress ) ),
                                                       m_sessionProperties ) );
      const IBindVariableHolder& variableHolder = *( bindVariables.back() );
      if ( ! variableHolder.succeeded() )
      {
        delete [] indicators;
        for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
          delete *iVariable;
        this->reset();
        return false;
      }
      dty = SQLT_BLOB;
      attributeAddress = variableHolder.bindAddress();
      attributeSize = variableHolder.size();
    }
    else if ( attributeType == typeid(coral::Date) )
    {
      bindVariables.push_back( new DateVariableHolder( *( static_cast<coral::Date*>( attributeAddress ) ) ) );
      const IBindVariableHolder& variableHolder = *( bindVariables.back() );
      if ( ! variableHolder.succeeded() )
      {
        delete [] indicators;
        for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
          delete *iVariable;
        this->reset();
        return false;
      }
      dty = SQLT_ODT;
      attributeAddress = variableHolder.bindAddress();
      attributeSize = variableHolder.size();
    }
    else if ( attributeType == typeid(coral::TimeStamp) )
    {
      bindVariables.push_back( new TimeVariableHolder( *( static_cast<coral::TimeStamp*>( attributeAddress ) ),
                                                       m_sessionProperties ) );
      const IBindVariableHolder& variableHolder = *( bindVariables.back() );
      if ( ! variableHolder.succeeded() )
      {
        delete [] indicators;
        for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
          delete *iVariable;
        this->reset();
        return false;
      }
      dty = SQLT_TIMESTAMP;
      attributeAddress = variableHolder.bindAddress();
      attributeSize = variableHolder.size();
    }
    else // No supported variable type
    {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << "Variable \"" << iAttribute->specification().typeName()
          << "\" can not be handled." << coral::MessageStream::endmsg;
      return false;
    }
    // Bind the variable
    OCIBind* ociBindHandle = 0;
    std::string theAttributeName = ":\"" + attributeName + "\"";
    status = OCIBindByName( m_ociStmtHandle,
                            &ociBindHandle,
                            m_sessionProperties->ociErrorHandle(),
                            reinterpret_cast< CONST text* >( theAttributeName.c_str() ),
                            ::strlen( theAttributeName.c_str() ),
                            attributeAddress,
                            attributeSize,
                            dty,
                            indicatorPointer,
                            0, 0, 0, 0,
                            OCI_DEFAULT );
    if ( status != OCI_SUCCESS )
    {
      OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Binding variable \"" + attributeName + "\" in \"" + m_sqlStatement + "\"" );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      if ( errorCodePtr != 0 ) *errorCodePtr = errorHandler.lastErrorCode();
      delete [] indicators;
      for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
        delete *iVariable;
      this->reset();
      return false;
    }
  }

  // Execute the actual statement
  coral::SimpleTimer timer;
  if ( m_statementStatistics ) timer.start();
  status = OCIStmtExecute( m_sessionProperties->ociSvcCtxHandle(),
                           m_ociStmtHandle,
                           m_sessionProperties->ociErrorHandle(),
                           ( statementType == OCI_STMT_SELECT ) ? 0 : 1,
                           0, 0, 0,
                           OCI_DEFAULT );
  if ( m_statementStatistics ) timer.stop();
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Executing statement \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    if ( errorCodePtr != 0 ) *errorCodePtr = errorHandler.lastErrorCode();
    delete [] indicators;
    for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
      delete *iVariable;
    this->reset();
    // Check if a duplicate entry has been attempted
    if ( statementType==OCI_STMT_INSERT && errorHandler.lastErrorCode()==1 )
      throw coral::DuplicateEntryInUniqueKeyException( m_sessionProperties->domainServiceName() );
    return false;
  }

  // Clean up the buffers
  if ( numberOfVariables > 0 )
  {
    delete [] indicators;
    for ( std::vector<IBindVariableHolder*>::iterator iVariable = bindVariables.begin(); iVariable != bindVariables.end(); ++iVariable ) 
      delete *iVariable;
  }

  // Monitor statistics
  if ( m_statementStatistics )
  {
    double idleTimeSeconds = timer.total() * 1e-6;
    if ( statementType != OCI_STMT_SELECT )
    {
      m_sessionProperties->monitoringService()->record( "oracle://" + m_sessionProperties->connectionString() + "/" + m_schemaName,
                                                        coral::monitor::Statement,
                                                        coral::monitor::Time,
                                                        m_statementStatistics->sqlStatement,
                                                        idleTimeSeconds );
    }
    else
    {
      m_statementStatistics->idleTime = idleTimeSeconds;
    }
  }
  return true;
}


long
coral::OracleAccess::OracleStatement::numberOfRowsProcessed() const
{
  ub4 result = 0;
  sword status = OCIAttrGet( m_ociStmtHandle, OCI_HTYPE_STMT,
                             &result, 0, OCI_ATTR_ROW_COUNT,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the number of rows processed by \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
  }
  return static_cast<long>( result );
}


const std::type_info*
coral::OracleAccess::OracleStatement::typeForOutputColumn( int columnId ) const
{
  //std::cout << "OracleStatement::typeForOutputColumn starting: " << columnId << std::endl; // debug bug #54968
  ub4 position = columnId + 1;

  // Get the column parameter
  void* temporaryPointer = 0;
  sword status = OCIParamGet( m_ociStmtHandle,
                              OCI_HTYPE_STMT,
                              m_sessionProperties->ociErrorHandle(),
                              &temporaryPointer,
                              position );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving an output parameter from \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return 0;
  }
  OCIParam* parameter = static_cast< OCIParam* >( temporaryPointer );

  // Retrieve the data type
  ub2 dataType = 0;
  status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                       &dataType, 0, OCI_ATTR_DATA_TYPE,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the data type of an output parameter in  \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return 0;
  }

  // Retrieve the data size
  sb4 attributeSize = 0;
  status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                       &attributeSize, 0, OCI_ATTR_DATA_SIZE,
                       m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the data size of an output parameter in \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return 0;
  }

  if ( dataType == SQLT_CHR ||
       dataType == SQLT_VCS ||
       dataType == SQLT_CLOB ||
       dataType == SQLT_LNG ||
       ( dataType == SQLT_AFC && attributeSize > 1 ) )
    return &( typeid(std::string) );
  else if( dataType == SQLT_AFC && attributeSize == 1 )
    return &( typeid(char) );
  else if ( dataType == SQLT_INT || dataType == SQLT_UIN )
    return &(typeid(int));
  else if ( dataType == SQLT_FLT )
    return &(typeid(double) );
  else if ( dataType == SQLT_IBFLOAT || dataType == SQLT_BFLOAT )
    return &(typeid(float));
  else if ( dataType == SQLT_IBDOUBLE || dataType == SQLT_BDOUBLE )
    return &(typeid(double));
  else if ( dataType == SQLT_BLOB )
    return &(typeid(coral::Blob));
  else if ( dataType == OCI_TYPECODE_DATE )
    return &(typeid(coral::Date));
  else if ( dataType == OCI_TYPECODE_TIMESTAMP )
    return &(typeid(coral::TimeStamp));
  else if ( dataType == SQLT_NUM )
  {
    // Retrieving the precision of the column
    //ub1 precision = 0; // here was bug #54968!
    sb2 precision = 0; // fix bug #54968!
    status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                         &precision, 0,
                         OCI_ATTR_PRECISION,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS )
    {
      OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the precision of an output parameter in \"" + m_sqlStatement + "\"" );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return 0;
    }
    // Retrieving the scale of the column
    sb1 scale = 0;
    status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                         &scale, 0,
                         OCI_ATTR_SCALE,
                         m_sessionProperties->ociErrorHandle() );
    if ( status != OCI_SUCCESS )
    {
      OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Retrieving the scale of an output parameter in \"" + m_sqlStatement + "\"" );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      return 0;
    }
    std::ostringstream os;
    if ( scale == -127 ) // This is a float
    {
      os << "FLOAT(" << static_cast<int>(precision) << ")";
    }
    else
    {
      os << "NUMBER";
      if ( precision != 0 )
      {
        os << "(" << static_cast<int>(precision);
        if ( scale != 0 )
        {
          os << "," << static_cast<int>(scale);
        }
        os << ")";
      }
    }
    const std::type_info* typeInfo = coral::AttributeSpecification::typeIdForName( m_sessionProperties->cppTypeForSqlType( os.str() ) );
    //std::cout << "OracleStatement::typeForOutputColumn exiting: " << (void*)typeInfo << ", " << typeInfo->name() << std::endl; // debug bug #54968
    return typeInfo;
  }
  else
  {
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << "Could not identify the output for column " << position << coral::MessageStream::endmsg;
    return 0;
  }
}


void
coral::OracleAccess::OracleStatement::defineOutput( coral::AttributeList& outputData )
{
  if ( outputData.size() == 0 ) return;
  m_outputData = &outputData;
  ub4 position = 1;
  m_indicators.resize( outputData.size() );
  m_outputVariables.reserve( outputData.size() );
  for ( coral::AttributeList::iterator iAttribute = outputData.begin(); iAttribute != outputData.end(); ++iAttribute, ++position )
  {
    const std::type_info& attributeType = iAttribute->specification().type();
    coral::Attribute& attribute = (*m_outputData)[position-1];
    attribute.shareData( *iAttribute );
    void* attributeAddress = const_cast< void * >( attribute.addressOfData() );
    // Check the type
    if ( attributeType == typeid(float) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<float>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(double) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<double>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(long double) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<long double>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(int) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<int>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(unsigned int) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<unsigned int>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(short) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<short>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(unsigned short) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<unsigned short>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(long) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<long>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(unsigned long) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<unsigned long>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(bool) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<bool>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(char) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<char>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(unsigned char) )
    {
      m_outputVariables.push_back( new OutputVariableHolder<unsigned char>( attributeAddress, m_sessionProperties ) );
    }
    else if ( attributeType == typeid(long long) )
    {
      m_outputVariables.push_back( new NumberOutputVariableHolder( attributeAddress, m_sessionProperties, true ) );
    }
    else if ( attributeType == typeid(unsigned long long) )
    {
      m_outputVariables.push_back( new NumberOutputVariableHolder( attributeAddress, m_sessionProperties, false ) );
    }
    else if ( attributeType == typeid(coral::Date) )
    {
      m_outputVariables.push_back( new DateOutputVariableHolder( *static_cast<coral::Date*>( attributeAddress ) ) );
    }
    else if ( attributeType == typeid(coral::TimeStamp) )
    {
      m_outputVariables.push_back( new TimeOutputVariableHolder( *static_cast<coral::TimeStamp*>( attributeAddress ), m_sessionProperties ) );
    }
    else if ( attributeType == typeid(coral::Blob) )
    {
      m_outputVariables.push_back( new BlobOutputVariableHolder( *static_cast<coral::Blob*>( attributeAddress ), m_sessionProperties ) );
    }
    else if ( attributeType == typeid(std::string) )
    {
      // Strings are special...
      // Get the type output type first
      void* temporaryPointer = 0;
      sword status = OCIParamGet( m_ociStmtHandle,
                                  OCI_HTYPE_STMT,
                                  m_sessionProperties->ociErrorHandle(),
                                  &temporaryPointer,
                                  position );
      if ( status != OCI_SUCCESS )
      {
        OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
        errorHandler.handleCase( status, "Retrieving the output parameter for variable : \"" + iAttribute->specification().name() + "\" in  \"" + m_sqlStatement + "\"" );
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        this->reset();
        // Throw an exception instead of returning false (fix bug #80099)
        throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                     errorHandler.message(),
                                     "OracleStatement" );
      }
      OCIParam* parameter = static_cast< OCIParam* >( temporaryPointer );
      // Retrieve the data size of the output column
      sb4 attributeSize = 0;
      status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                           &attributeSize,
                           0, OCI_ATTR_DATA_SIZE,
                           m_sessionProperties->ociErrorHandle() );
      if ( status != OCI_SUCCESS )
      {
        OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
        errorHandler.handleCase( status, "Retrieving the output parameter data size for variable : \"" + iAttribute->specification().name() + "\" in \"" + m_sqlStatement + "\"" );
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        this->reset();
        // Throw an exception instead of returning false (fix bug #80099)
        throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                     errorHandler.message(),
                                     "OracleStatement" );
      }

      // Retrieve the data type of the output column
      ub2 attributeSType;
      status = OCIAttrGet( parameter, OCI_DTYPE_PARAM,
                           &attributeSType, 0, OCI_ATTR_DATA_TYPE,
                           m_sessionProperties->ociErrorHandle() );
      if ( status != OCI_SUCCESS )
      {
        OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
        errorHandler.handleCase( status, "Retrieving the output parameter data type for variable : \"" + iAttribute->specification().name() + "\" in \"" + m_sqlStatement + "\"" );
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
        this->reset();
        // Throw an exception instead of returning false (fix bug #80099)
        throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                     errorHandler.message(),
                                     "OracleStatement" );
      }

      // Differentiate on the type
      if ( attributeSType == OCI_TYPECODE_VARCHAR2 ||
           attributeSType == OCI_TYPECODE_VARCHAR ||
           attributeSType == OCI_TYPECODE_CHAR )
      {
        ++attributeSize;
        m_outputVariables.push_back( new StringOutputVariableHolder( *static_cast<std::string*>( attributeAddress ), attributeSize) );
      }
      else if ( attributeSType == SQLT_LNG ) {
        m_outputVariables.push_back( new LongStringOutputVariableHolder( *static_cast<std::string*>( attributeAddress ) ) );
      }
      else if ( attributeSType == OCI_TYPECODE_CLOB )
      {
        m_outputVariables.push_back( new ClobOutputVariableHolder( *static_cast<std::string*>( attributeAddress ), m_sessionProperties ) );
      }
      else
      {
        coral::MessageStream log( m_sessionProperties->domainServiceName() );
        log << coral::Error << "Variable type missmatch" << coral::MessageStream::endmsg;
        // Throw an exception instead of returning false (fix bug #80099)
        throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                     "Variable type missmatch",
                                     "OracleStatement" );
      }
    }
    else
    {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      std::stringstream msg;
      msg << "Variable type \"" << iAttribute->specification().typeName() << "\" can not be handled";
      log << coral::Error << msg.str() << coral::MessageStream::endmsg;
      // Throw an exception instead of returning false (fix bug #80099)
      throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                   msg.str(),
                                   "OracleStatement" );
    }
    const IOutputVariableHolder& outputVariable = *( m_outputVariables.back() );
    if ( ! outputVariable.suceeded() )
    {
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      std::stringstream msg;
      msg << "Could not define a " << iAttribute->specification().typeName() << " variable";
      log << coral::Error << msg.str() << coral::MessageStream::endmsg;
      this->reset();
      // Throw an exception instead of returning false (fix bug #80099)
      throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                   msg.str(),
                                   "OracleStatement" );
    }

    // Perform the actual defining
    OCIDefine* definePointer = 0;
    sword status = OCIDefineByPos( m_ociStmtHandle,
                                   &definePointer,
                                   m_sessionProperties->ociErrorHandle(),
                                   position,
                                   outputVariable.defineAddress(),
                                   outputVariable.size(),
                                   outputVariable.dty(),
                                   &( m_indicators[ position - 1 ] ),
                                   outputVariable.lengthPointer(),
                                   0,
                                   OCI_DEFAULT );
    if ( status != OCI_SUCCESS )
    {
      OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
      errorHandler.handleCase( status, "Defining the output for variable : \"" + iAttribute->specification().name() + "\" in \"" + m_sqlStatement + "\"" );
      coral::MessageStream log( m_sessionProperties->domainServiceName() );
      log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
      this->reset();
      // Throw an exception instead of returning false (fix bug #80099)
      throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                   errorHandler.message(),
                                   "OracleStatement" );
    }
  }
}


bool
coral::OracleAccess::OracleStatement::setNumberOfPrefetchedRows( unsigned int numberOfRows )
{
  ub4 attributeValue = numberOfRows;
  sword status = OCIAttrSet( m_ociStmtHandle, OCI_HTYPE_STMT,
                             &attributeValue, sizeof(ub4), OCI_ATTR_PREFETCH_ROWS,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Setting the number of prefetched rows in \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    this->reset();
    return false;
  }
  return true;
}


bool
coral::OracleAccess::OracleStatement::setCacheSize( unsigned int sizeInMB )
{
  //std::cout << "OracleStatement::setCacheSize starting" << std::endl; // debug bug #54968
  ub4 attributeValue = sizeInMB * 1000000; // Fix bug #64215
  sword status = OCIAttrSet( m_ociStmtHandle, OCI_HTYPE_STMT,
                             &attributeValue, sizeof(ub4), OCI_ATTR_PREFETCH_MEMORY,
                             m_sessionProperties->ociErrorHandle() );
  //std::cout << "OracleStatement::setCacheSize checking status" << std::endl; // debug bug #54968
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Setting the size of the cache for prefetched rows in \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    this->reset();
    return false;
  }
  return true;
}


bool
coral::OracleAccess::OracleStatement::fetchNext()
{
  //std::cout << "Statement " << this << ": fetchNext using SessionProperties " << &m_sessionProperties << std::endl; // debug bug #73334
  //std::cout << "Statement " << this << ": m_sessionProperties->ociErrorHandle() may crash?" << std::endl; // debug bug #73334
  //m_sessionProperties->ociErrorHandle();
  //std::cout << "Statement " << this << ": m_sessionProperties->ociErrorHandle() did not crash" << std::endl; // debug bug #73334
  //std::cout << "Statement " << this << ": &(m_sessionProperties->connectionProperties()) may crash?" << std::endl; // debug bug #73334
  //std::cout << "Statement " << this << ": &(m_sessionProperties->connectionProperties()) is " <<  &(m_sessionProperties->connectionProperties()) << std::endl; // debug bug #73334
  //std::cout << "Statement " << this << ": m_sessionProperties->connectionProperties().domainServiceName() may crash?" << std::endl; // debug bug #73334
  //m_sessionProperties->connectionProperties().domainServiceName();
  //std::cout << "Statement " << this << ": m_sessionProperties->connectionProperties().domainServiceName() did not crash" << std::endl; // debug bug #73334

  //boost::mutex::scoped_lock lock( m_sessionProperties->connectionProperties().connectionMutex() );

  // Fix bug #75094 (cursor::next should fail if transaction is not active)
  if( !m_sessionProperties->isTransactionActive() )
    throw coral::QueryException( m_sessionProperties->domainProperties().service()->name(), "Transaction is not active", "fetchNext()" );

  coral::SimpleTimer timer;
  if ( m_statementStatistics ) timer.start();
  sword status = OCIStmtFetch2( m_ociStmtHandle,
                                m_sessionProperties->ociErrorHandle(),
                                1, OCI_FETCH_NEXT,
                                0, OCI_DEFAULT );
  if ( status == OCI_NO_DATA )
  {
    if ( m_statementStatistics )
    {
      double idleTimeSeconds = timer.sample() * 1e-6;
      m_statementStatistics->idleTime += idleTimeSeconds;
      m_sessionProperties->monitoringService()->record( "oracle://" + m_sessionProperties->connectionString() + "/" + m_schemaName,
                                                        coral::monitor::Statement,
                                                        coral::monitor::Time,
                                                        m_statementStatistics->sqlStatement,
                                                        m_statementStatistics->idleTime );
    }
    return false;
  }
  if ( status != OCI_SUCCESS )
  {
    //std::cout << "Statement " << this << ": m_ociStmtHandle is invalid..." << std::endl; // debug bug #79983
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Fetching the next row from \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    if ( m_statementStatistics )
    {
      double idleTimeSeconds = timer.sample() * 1e-6;
      m_statementStatistics->idleTime += idleTimeSeconds;
      m_sessionProperties->monitoringService()->record( "oracle://" + m_sessionProperties->connectionString() + "/" + m_schemaName,
                                                        coral::monitor::Statement,
                                                        coral::monitor::Time,
                                                        m_statementStatistics->sqlStatement,
                                                        m_statementStatistics->idleTime );
    }
    // Throw an exception instead of returning false (fix bug #78178)
    throw coral::QueryException( m_sessionProperties->domainServiceName(),
                                 errorHandler.message(),
                                 "OracleStatement" );
  }
  //std::cout << "Statement " << this << ": fetchNext copyOutput" << std::endl; // debug bug #73334
  bool result = this->copyOutput();
  if ( m_statementStatistics )
  {
    timer.stop();
    double idleTimeSeconds = timer.total() * 1e-6;
    m_statementStatistics->idleTime += idleTimeSeconds;
    if ( ! result )
    {
      m_sessionProperties->monitoringService()->record( "oracle://" + m_sessionProperties->connectionString() + "/" + m_schemaName,
                                                        coral::monitor::Statement,
                                                        coral::monitor::Time,
                                                        m_statementStatistics->sqlStatement,
                                                        m_statementStatistics->idleTime );
    }
  }
  return result;
}


bool
coral::OracleAccess::OracleStatement::copyOutput()
{
  int indicator = 0;
  std::vector<IOutputVariableHolder* >::iterator iOutputVariable = m_outputVariables.begin();
  for ( coral::AttributeList::iterator iAttribute = m_outputData->begin(); iAttribute != m_outputData->end(); ++iAttribute, ++indicator, ++iOutputVariable ) 
  {
    if ( m_indicators[indicator] == -1 )
    {
      iAttribute->setNull( true );
    }
    else
    {
      if ( ! (*iOutputVariable)->copyData() )
      {
        this->reset();
        return false;
      }
      iAttribute->setNull( false );
    }
  }
  return true;
}


unsigned int
coral::OracleAccess::OracleStatement::currentRowNumber() const
{
  ub4 result = 0;
  sword status = OCIAttrGet( m_ociStmtHandle, OCI_HTYPE_STMT,
                             &result, 0, OCI_ATTR_ROW_COUNT,
                             m_sessionProperties->ociErrorHandle() );
  if ( status != OCI_SUCCESS )
  {
    OracleErrorHandler errorHandler( m_sessionProperties->ociErrorHandle() );
    errorHandler.handleCase( status, "Retrieving the row number from \"" + m_sqlStatement + "\"" );
    coral::MessageStream log( m_sessionProperties->domainServiceName() );
    log << coral::Error << errorHandler.message() << coral::MessageStream::endmsg;
    return 0;
  }
  return result;
}
