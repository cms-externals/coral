// $Id: TableDescriptionProxy.cpp,v 1.8.2.1 2010/12/20 09:10:09 avalassi Exp $

// Include files
#include "CoralServerBase/ICoralFacade.h"
#include "CoralServerBase/InternalErrorException.h"

// Local include files
#include "ConnectionProperties.h"
#include "SessionProperties.h"
#include "TableDescriptionProxy.h"

// Namespace
using namespace coral::CoralAccess;

//-----------------------------------------------------------------------------

TableDescriptionProxy::TableDescriptionProxy( const SessionProperties& sessionProperties,
                                              const std::string& schemaName,
                                              const std::string& tableName,
                                              bool isView )
  : m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_tableName( tableName )
  , m_isView( isView )
  , m_pDesc( 0 )
  , m_viewDefinition( "" )
{
  if ( m_schemaName == "" )
    throw InternalErrorException( "PANIC! Invalid schema name ''",
                                  "TableDescriptionProxy::TableDescriptionProxy",
                                  "coral::CoralAccess" );
}

//-----------------------------------------------------------------------------

TableDescriptionProxy::~TableDescriptionProxy()
{
  if ( m_pDesc ) delete m_pDesc;
  m_pDesc = 0;
}

//-----------------------------------------------------------------------------

std::string TableDescriptionProxy::name() const
{
  return m_tableName;
}

//-----------------------------------------------------------------------------

const coral::ITableDescription& TableDescriptionProxy::proxiedTableDescription() const
{
  if ( ! m_pDesc )
  {
    if ( !m_isView )
    {
      m_pDesc = new TableDescription( m_sessionProperties.connectionProperties().facade().fetchTableDescription
                                      ( m_sessionProperties.sessionID(), m_schemaName, m_tableName ) );
    }
    else
    {
      std::pair< TableDescription, std::string > viewDesc =
        m_sessionProperties.connectionProperties().facade().fetchViewDescription
        ( m_sessionProperties.sessionID(), m_schemaName, m_tableName );
      m_pDesc = new TableDescription( viewDesc.first );
      m_viewDefinition = viewDesc.second;
    }
    if ( m_tableName != m_pDesc->name() )
      throw InternalErrorException( std::string( "PANIC! " ) + ( m_isView ? "Table" : "View" ) + " name mismatch",
                                    "TableDescriptionProxy::proxiedTableDescription()",
                                    "coral::CoralAccess" );
  }
  return *m_pDesc;
}

//-----------------------------------------------------------------------------

const std::string& TableDescriptionProxy::viewDefinition() const
{
  if ( !m_isView )
    throw InternalErrorException( "PANIC! Table '" + name() + "' is not a view",
                                  "TableDescriptionProxy::viewDefinition()",
                                  "coral::CoralAccess" );
  if ( ! m_pDesc ) proxiedTableDescription();  // Load the definition if needed
  return m_viewDefinition;
}

//-----------------------------------------------------------------------------
