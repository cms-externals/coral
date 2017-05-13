#include "View.h"
#include "ISessionProperties.h"
#include "DomainProperties.h"
#include "ErrorHandler.h"
#include "Statement.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/ITypeConverter.h"
#include "CoralBase/Blob.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralKernel/Service.h"
#include "CoralBase/MessageStream.h"

coral::FrontierAccess::View::View( const coral::FrontierAccess::ISessionProperties& sessionProperties, const std::string& viewName )
  : m_sessionProperties( sessionProperties )
  , m_definitionString( "" )
  , m_description( new coral::TableDescription( m_sessionProperties.domainProperties().service()->name() ) )
{
  m_description->setName( viewName );
}

coral::ITablePrivilegeManager& coral::FrontierAccess::View::privilegeManager()
{
  throw coral::InvalidOperationInReadOnlyModeException( m_sessionProperties.domainProperties().service()->name(), "FrontierAccess::View::privilegeManager" );
}

const coral::ITableDescription& coral::FrontierAccess::View::description() const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::FrontierAccess::View*>( this )->refreshDescription();
  return *m_description;
}

coral::FrontierAccess::View::~View()
{
  delete m_description;
}

std::string coral::FrontierAccess::View::name() const
{
  return m_description->name();
}

std::string coral::FrontierAccess::View::definition() const
{
  if ( m_definitionString.empty() )
    const_cast<coral::FrontierAccess::View*>( this )->refreshDefinition();
  return m_definitionString;
}

int coral::FrontierAccess::View::numberOfColumns() const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::FrontierAccess::View*>( this )->refreshDescription();
  return m_description->numberOfColumns();
}

const coral::IColumn& coral::FrontierAccess::View::column( int index ) const
{
  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::FrontierAccess::View*>( this )->refreshDescription();
  return m_description->columnDescription( index );
}

void coral::FrontierAccess::View::refreshDescription()
{
  // Retrieving the describe parameter handle
  // Retrieving the column list
  // Retrieving the number of columns

  //   for ( ub4 iColumn = 1; iColumn <= numberOfColumns; ++iColumn )
  //   {
  // Retrieving the column parameter
  // Retrieving the column name
  // Retrieving the column type
  // Retrieving the precision of the column
  // Retrieving the scale of the column
  // Retrieving the data size of the column
  // Retrieving the nullness of the column
  //   }
}

void coral::FrontierAccess::View::refreshDefinition()
{
  std::ostringstream os;
  os << "SELECT TEXT FROM ALL_VIEWS WHERE VIEW_NAME='"
     << this->description().name()
     << "' AND OWNER='" << m_sessionProperties.schemaName() << "'";

  coral::FrontierAccess::Statement statement( m_sessionProperties, os.str() );

  if ( ! statement.execute( coral::AttributeList(), false ) )
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not read the definition of a view", "View::refreshDefinition" );

  coral::AttributeList* output = new coral::AttributeList;
  output->extend( "TEXT", typeid(std::string) );
  output->begin()->data<std::string>() = "";
  statement.defineOutput( *output );
  statement.fetchNext();
  m_definitionString = output->begin()->data<std::string>();
  delete output;
  if ( m_definitionString.empty() )
  {
    throw coral::SchemaException( m_sessionProperties.domainProperties().service()->name(), "Could not read the definition of a view", "View::refreshDefinition" );
  }

  // Remove the parentheses
  while ( m_definitionString[0] == ' ' || m_definitionString[0] == '\t' || m_definitionString[0] == '\n' || m_definitionString[0] == '\r' )
  {
    m_definitionString = m_definitionString.substr( 1 );
  }

  std::string::size_type iPos = m_definitionString.size() - 1;

  while ( m_definitionString[iPos] == ' ' || m_definitionString[iPos] == '\t' || m_definitionString[iPos] == '\n' || m_definitionString[iPos] == '\r' )
  {
    m_definitionString = m_definitionString.substr( 0, iPos );
    --iPos;
  }

  if ( m_definitionString[0] == '(' && m_definitionString[iPos] == ')' )
  {
    m_definitionString = m_definitionString.substr( 1, iPos - 1 );

    while ( m_definitionString[0] == ' ' || m_definitionString[0] == '\t' || m_definitionString[0] == '\n' || m_definitionString[0] == '\r' )
    {
      m_definitionString = m_definitionString.substr( 1 );
    }

    iPos = m_definitionString.size() - 1;

    while ( m_definitionString[iPos] == ' ' || m_definitionString[iPos] == '\t' || m_definitionString[iPos] == '\n' || m_definitionString[iPos] == '\r' )
    {
      m_definitionString = m_definitionString.substr( 0, iPos );
      --iPos;
    }
  }
}
