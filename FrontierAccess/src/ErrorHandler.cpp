#include "ErrorHandler.h"
#include <sstream>

coral::FrontierAccess::ErrorHandler::ErrorHandler() :
  m_error( false ),
  m_lastMessage( "" ),
  m_lastErrorCode( 0 )
{
}

coral::FrontierAccess::ErrorHandler::~ErrorHandler()
{
}

void coral::FrontierAccess::ErrorHandler::handleCase( int /* status */ , const std::string& /* context */ )
{
}

bool coral::FrontierAccess::ErrorHandler::isError() const
{
  return m_error;
}

const std::string coral::FrontierAccess::ErrorHandler::message() const
{
  return m_lastMessage;
}

int coral::FrontierAccess::ErrorHandler::lastErrorCode() const
{
  return m_lastErrorCode;
}
