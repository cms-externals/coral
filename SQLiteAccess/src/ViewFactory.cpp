#include <iostream>
#include "ViewFactory.h"
#include "View.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "SQLiteStatement.h"

#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/SchemaException.h"

#include "CoralKernel/Service.h"

coral::SQLiteAccess::ViewFactory::ViewFactory( boost::shared_ptr<const SessionProperties> properties ) :
  coral::SQLiteAccess::QueryDefinition( properties )
{

}


coral::SQLiteAccess::ViewFactory::~ViewFactory()
{

}


coral::IView&
coral::SQLiteAccess::ViewFactory::create( const std::string& viewName )
{

  //std::cout<<"SQLiteAccess::ViewFactory::create "<<viewName<<std::endl;
  if ( this->sessionProperties()->schema().existsView( viewName ) )
    throw coral::ViewAlreadyExistingException( this->sessionProperties()->domainProperties().service()->name(),viewName );

  std::string sqlStatement =
    "CREATE VIEW \"" + viewName + "\" AS " + this->sqlFragment();
  coral::SQLiteAccess::SQLiteStatement statement( this->sessionProperties() );
  statement.prepare( sqlStatement );
  if ( !statement.bind( this->bindData()) || ! statement.execute() )
    throw coral::SchemaException( this->sessionProperties()->domainProperties().service()->name(),
                                  "Could not create view " + viewName,
                                  "IViewFactory::create" );
  // Get the view handle
  coral::IView& view = this->sessionProperties()->schema().viewHandle(viewName);
  return view;
}


coral::IView&
coral::SQLiteAccess::ViewFactory::createOrReplace( const std::string& viewName )
{

  //std::cout<<"SQLiteAccess::ViewFactory::createOrReplace "<<viewName<<std::endl;
  if( this->sessionProperties()->schema().existsView( viewName ) ) {
    this->sessionProperties()->schema().dropView(viewName);
  }
  return this->create(viewName);
}
