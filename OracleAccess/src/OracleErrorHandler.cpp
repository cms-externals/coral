#include <sstream>
#include "oci.h"

#include "CoralBase/MessageStream.h"
#include "OracleErrorHandler.h"

coral::OracleAccess::OracleErrorHandler::OracleErrorHandler( OCIError* ociErrorHandle ) :
  m_ociErrorHandle( ociErrorHandle ),
  m_error( false ),
  m_lastMessage( "" ),
  m_lastErrorCode( 0 )
{
}

coral::OracleAccess::OracleErrorHandler::~OracleErrorHandler()
{
}

#ifdef CORAL240CN
void
coral::OracleAccess::OracleErrorHandler::handleCase( sword status,
                                                     const std::string& context,
                                                     bool printToLog )
#else
  void
coral::OracleAccess::OracleErrorHandler::handleCase( sword status,
                                                     const std::string& context )
#endif
{
  text errbuf[512];
  m_error = true;
  switch (status)
  {
  case OCI_SUCCESS:
    m_error = false;
    break;
  case OCI_SUCCESS_WITH_INFO:
    {
      OCIErrorGet( m_ociErrorHandle, 1, 0, &m_lastErrorCode,
                   errbuf, sizeof(errbuf), OCI_HTYPE_ERROR );
      std::ostringstream os;
      os << errbuf;
      std::string serrbuf = os.str();
      while ( ! serrbuf.empty() &&
              ( serrbuf[ serrbuf.size() - 1 ] == '\n' ||
                serrbuf[ serrbuf.size() - 1 ] == '\r' ||
                serrbuf[ serrbuf.size() - 1 ] == '\t' ||
                serrbuf[ serrbuf.size() - 1 ] == ' ' ) ) serrbuf = serrbuf.substr( 0, serrbuf.size() - 1 );
      m_lastMessage = serrbuf + " (" + context + ")";
      m_error = false;
    }
    break;
  case OCI_NEED_DATA:
    m_lastMessage = "OCI needs data (" + context + ")";
    break;
  case OCI_NO_DATA:
    m_lastMessage = "OCI found no data (" + context + ")";
    break;
  case OCI_ERROR:
    {
      OCIErrorGet( m_ociErrorHandle, 1, 0, &m_lastErrorCode,
                   errbuf, sizeof(errbuf), OCI_HTYPE_ERROR );
      std::ostringstream os;
      os << errbuf;
      std::string serrbuf = os.str();
      while ( ! serrbuf.empty() &&
              ( serrbuf[ serrbuf.size() - 1 ] == '\n' ||
                serrbuf[ serrbuf.size() - 1 ] == '\r' ||
                serrbuf[ serrbuf.size() - 1 ] == '\t' ||
                serrbuf[ serrbuf.size() - 1 ] == ' ' ) ) serrbuf = serrbuf.substr( 0, serrbuf.size() - 1 );
      m_lastMessage = serrbuf + " (" + context + ")";
    }
    break;
  case OCI_INVALID_HANDLE:
    m_lastMessage = "Supplied invalid OCI handle (" + context + ")";
    break;
  case OCI_STILL_EXECUTING:
    m_lastMessage = "OCI is still executing (" + context + ")";
    m_error = false;
    break;
  default:
    break;
  }
#ifdef CORAL240CN
  // Print the error message to log if requested
  if( m_error && printToLog )
  {
    coral::MessageStream log( context );
    log << coral::Error << m_lastMessage << coral::MessageStream::endmsg;
  }
#endif
}

bool
coral::OracleAccess::OracleErrorHandler::isError() const
{
  return m_error;
}

const std::string&
coral::OracleAccess::OracleErrorHandler::message() const
{
  return m_lastMessage;
}


sb4
coral::OracleAccess::OracleErrorHandler::lastErrorCode() const
{
  return m_lastErrorCode;
}
