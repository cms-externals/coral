// $Id: ViewFactory.cpp,v 1.6 2011/03/22 10:29:55 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralKernel/Service.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/SchemaException.h"

#include "DomainProperties.h"
#include "ISessionProperties.h"
#include "Statement.h"
#include "ViewFactory.h"

coral::MySQLAccess::ViewFactory::ViewFactory( const coral::MySQLAccess::ISessionProperties& properties )
  : coral::MySQLAccess::QueryDefinition( properties )
{
}

coral::MySQLAccess::ViewFactory::~ViewFactory()
{
}

coral::IView&
coral::MySQLAccess::ViewFactory::create( const std::string& /*viewName*/ )
{
  throw coral::Exception( "Views are not fully supported in MySQLAccess (bug #36512)", m_properties.domainProperties().service()->name(), "ISchema::dropIfExistsView" );
  /*
  if ( this->sessionProperties().schema().existsView( viewName ) )
    throw coral::ViewAlreadyExistingException( this->sessionProperties().domainProperties().service()->name(), viewName );
  std::string sqlStatement = "CREATE VIEW " + this->sessionProperties().schemaName() + ".\"" + viewName + "\" AS ( " + this->sqlFragment() + " )";
  coral::MySQLAccess::Statement statement( this->sessionProperties(), sqlStatement );
  if ( ! statement.execute( this->bindData() ) )
    throw coral::SchemaException( this->sessionProperties().domainProperties().service()->name(), "Could not create view \"" + viewName + "\"", "IViewFactory::create" );
  return this->sessionProperties().schema().viewHandle( viewName );
  */
}


coral::IView&
coral::MySQLAccess::ViewFactory::createOrReplace( const std::string& /*viewName(*/ )
{
  throw coral::Exception( "Views are not fully supported in MySQLAccess (bug #36512)", m_properties.domainProperties().service()->name(), "ISchema::dropIfExistsView" );
  /*
  std::string sqlStatement = "CREATE OR REPLACE VIEW " + this->sessionProperties().schemaName() + ".\"" + viewName + "\" AS ( " + this->sqlFragment() + " )";
  coral::MySQLAccess::Statement statement( this->sessionProperties(), sqlStatement );
  if ( ! statement.execute( this->bindData() ) )
    throw coral::SchemaException( this->sessionProperties().domainProperties().service()->name(), "Could not create view \"" + viewName + "\"", "IViewFactory::create" );
  return this->sessionProperties().schema().viewHandle( viewName );
  */
}
