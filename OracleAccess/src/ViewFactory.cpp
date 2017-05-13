#include "CoralKernel/Service.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "OracleStatement.h"
#include "SessionProperties.h"
#include "ViewFactory.h"


coral::OracleAccess::ViewFactory::ViewFactory( boost::shared_ptr<const SessionProperties> properties,
                                               const std::string& schemaName ) :
  coral::OracleAccess::QueryDefinition( properties, schemaName )
{
}


coral::OracleAccess::ViewFactory::~ViewFactory()
{
}


coral::IView&
coral::OracleAccess::ViewFactory::create( const std::string& viewName )
{
  if ( this->sessionProperties()->existsView( m_schemaName, viewName ) )
    throw coral::ViewAlreadyExistingException( this->sessionProperties()->domainServiceName(), viewName );
  std::string sqlStatement =
    "CREATE VIEW " + m_schemaName + ".\"" + viewName + "\" AS ( " + this->sqlFragment() + " )";
  coral::OracleAccess::OracleStatement statement( this->sessionProperties(),
                                                  m_schemaName,
                                                  sqlStatement );
  if ( ! statement.execute( this->bindData() ) )
    throw coral::SchemaException( this->sessionProperties()->domainServiceName(),
                                  "Could not create view \"" + viewName + "\"",
                                  "IViewFactory::create" );
  return this->sessionProperties()->viewHandle( m_schemaName, viewName );
}


coral::IView&
coral::OracleAccess::ViewFactory::createOrReplace( const std::string& viewName )
{
  std::string sqlStatement =
    "CREATE OR REPLACE VIEW " + m_schemaName + ".\"" + viewName + "\" AS ( " + this->sqlFragment() + " )";
  coral::OracleAccess::OracleStatement statement( this->sessionProperties(),
                                                  m_schemaName,
                                                  sqlStatement );
  if ( ! statement.execute( this->bindData() ) )
    throw coral::SchemaException( this->sessionProperties()->domainServiceName(),
                                  "Could not create view \"" + viewName + "\"",
                                  "IViewFactory::create" );
  return this->sessionProperties()->viewHandle( m_schemaName, viewName );
}
