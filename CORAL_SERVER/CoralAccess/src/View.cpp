// $Id: View.cpp,v 1.3.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include <iostream>
#include "CoralServerBase/InternalErrorException.h"
#include "CoralServerBase/NotImplemented.h"

// Local include files
#include "View.h"
#include "logger.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

View::View( const SessionProperties& sessionProperties,
            const std::string& schemaName,
            const std::string& viewName )
  : m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_description( new TableDescriptionProxy( sessionProperties, schemaName, viewName, true ) )
{
  logger << "Create View '" << viewName << "'" << endlog;
  if ( m_schemaName == "" )
    throw InternalErrorException( "PANIC! Invalid schema name ''",
                                  "View::View()",
                                  "coral::CoralAccess" );
}

//-----------------------------------------------------------------------------

View::~View()
{
  if ( m_description ) logger << "Delete View '" << m_description->name() << "'" << endlog;
  else logger << "Delete View (WARNING: invalid description)" << endlog;
  if ( m_description ) delete m_description;
  m_description = 0;
}

//-----------------------------------------------------------------------------

std::string View::name() const
{
  if ( ! m_description )
    throw InternalErrorException( "PANIC! Invalid view description",
                                  "View::name",
                                  "coral::CoralAccess" );
  return m_description->name();
}

//-----------------------------------------------------------------------------

std::string View::definition() const
{
  if ( ! m_description )
    throw InternalErrorException( "PANIC! Invalid view description",
                                  "View::definition",
                                  "coral::CoralAccess" );
  return m_description->viewDefinition();
}

//-----------------------------------------------------------------------------

int View::numberOfColumns() const
{
  if ( ! m_description )
    throw InternalErrorException( "PANIC! Invalid view description",
                                  "View::numberOfColumns",
                                  "coral::CoralAccess" );
  return m_description->numberOfColumns();
}

//-----------------------------------------------------------------------------

const coral::IColumn& View::column( int index ) const
{
  if ( ! m_description )
    throw InternalErrorException( "PANIC! Invalid view description",
                                  "View::column",
                                  "coral::CoralAccess" );
  return m_description->columnDescription( index );
}

//-----------------------------------------------------------------------------

coral::ITablePrivilegeManager& View::privilegeManager()
{
  throw NotImplemented("View::privilegeManager");
}

//-----------------------------------------------------------------------------
