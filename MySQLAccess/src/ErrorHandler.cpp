// $Id: ErrorHandler.cpp,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <sstream>

#include "ErrorHandler.h"

coral::MySQLAccess::ErrorHandler::ErrorHandler()
  : m_error( false )
  , m_lastMessage( "" )
  , m_lastErrorCode( 0 /* Means success */)
  , m_stmtError( false )
  , m_stmtLastMessage( "" )
  , m_stmtLastErrorCode( 0 /* Means success */)
{
}

coral::MySQLAccess::ErrorHandler::~ErrorHandler()
{
}

void coral::MySQLAccess::ErrorHandler::handleCase( MYSQL* connection, const std::string& context )
{
  m_lastErrorCode            = mysql_errno( connection );
  const char* lastErrMsg     = mysql_error( connection );

  if( m_lastErrorCode != 0 )
    m_error = true;

  std::stringstream ss;
  ss << context << ": MySQL errno:" << m_lastErrorCode;
  m_lastMessage += ss.str();

  if( lastErrMsg[0] != '\0' )
  {
    m_lastMessage += ": "; m_lastMessage += lastErrMsg;
  }
}

void coral::MySQLAccess::ErrorHandler::handleCase( MYSQL* connection, const std::string& context, const char* fileName, unsigned int lineNumber )
{
  std::stringstream ss;
  ss << fileName << ":" << lineNumber << ":" << context;
  this->handleCase( connection, ss.str() );
}

bool coral::MySQLAccess::ErrorHandler::isError() const
{
  return m_error;
}

std::string coral::MySQLAccess::ErrorHandler::message() const
{
  return m_lastMessage;
}

unsigned int coral::MySQLAccess::ErrorHandler::lastErrorCode() const
{
  return m_lastErrorCode;
}

#if (  MYSQL_VERSION_ID > 40100 )
void coral::MySQLAccess::ErrorHandler::handleCase( MYSQL_STMT* connection, const std::string& context )
{
  m_stmtLastErrorCode            = mysql_stmt_errno( connection );
  const char* stmtLastErrMsg     = mysql_stmt_error( connection );

  if( m_stmtLastErrorCode != 0 )
    m_stmtError = true;

  std::stringstream ss;
  ss << context << ": MySQL statement errno:" << m_stmtLastErrorCode;
  m_stmtLastMessage += ss.str();

  if( stmtLastErrMsg[0] != '\0' )
  {
    m_stmtLastMessage += ": "; m_stmtLastMessage += stmtLastErrMsg;
  }
}

void coral::MySQLAccess::ErrorHandler::handleCase( MYSQL_STMT* connection, const std::string& context, const char* fileName, unsigned int lineNumber )
{
  std::stringstream ss;
  ss << fileName << ":" << lineNumber << ":" << context;
  this->handleCase( connection, ss.str() );
}
#endif

bool coral::MySQLAccess::ErrorHandler::isStmtError() const
{
  return m_stmtError;
}

std::string coral::MySQLAccess::ErrorHandler::stmtMessage() const
{
  return m_stmtLastMessage;
}

unsigned int coral::MySQLAccess::ErrorHandler::stmtLastErrorCode() const
{
  return m_stmtLastErrorCode;
}
