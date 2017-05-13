#include <iostream>
#include "View.h"
#include "SessionProperties.h"
#include "DomainProperties.h"
#include "PrivilegeManager.h"
#include "SQLiteStatement.h"

#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"
#include "RelationalAccess/ITypeConverter.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/MessageStream.h"

#include "CoralKernel/Service.h"

coral::SQLiteAccess::View::View( boost::shared_ptr<const SessionProperties> properties,const std::string& viewName) :
  m_properties( properties ),
  m_definitionString( "" ),
  m_description( new coral::TableDescription( m_properties->domainProperties().service()->name()) ) ,
  m_privilegeManager( new coral::SQLiteAccess::PrivilegeManager( m_properties,m_properties->schemaName()+"."+viewName) )
{

  m_description->setName( viewName );
}


coral::ITablePrivilegeManager&
coral::SQLiteAccess::View::privilegeManager()
{

  if ( ! m_properties->isTransactionActive() )
    throw coral::TransactionNotActiveException( m_properties->domainProperties().service()->name(),"IView::privilegeManager" );
  if ( m_properties->isReadOnly() )
    throw coral::InvalidOperationInReadOnlyModeException( m_properties->domainProperties().service()->name(), "IView::privilegeManager" );
  if ( m_properties->isTransactionReadOnly() )
    throw coral::InvalidOperationInReadOnlyTransactionException( m_properties->domainProperties().service()->name(), "IView::privilegeManager" );
  return *m_privilegeManager;
}

const coral::ITableDescription&
coral::SQLiteAccess::View::description() const
{

  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::SQLiteAccess::View*>( this )->refreshDescription();
  return *m_description;
}


coral::SQLiteAccess::View::~View()
{

  delete m_privilegeManager;
  delete m_description;
}


std::string
coral::SQLiteAccess::View::name() const
{

  return m_description->name();
}


std::string
coral::SQLiteAccess::View::definition() const
{

  if ( m_definitionString.empty() )
    const_cast<coral::SQLiteAccess::View*>( this )->refreshDefinition();
  return m_definitionString;
}


int
coral::SQLiteAccess::View::numberOfColumns() const
{

  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::SQLiteAccess::View*>( this )->refreshDescription();
  return m_description->numberOfColumns();
}


const coral::IColumn&
coral::SQLiteAccess::View::column( int index ) const
{

  if ( m_description->numberOfColumns() == 0 )
    const_cast<coral::SQLiteAccess::View*>( this )->refreshDescription();
  return m_description->columnDescription( index );
}


void
coral::SQLiteAccess::View::refreshDescription()
{

  coral::SQLiteAccess::SQLiteStatement stmt(m_properties);
  ////////////////////////////////////////////////////////////////////
  //result structure
  // 0   1    2    3          4       5
  //cid name type notnull dflt_value  pk
  ////////////////////////////////////////////////////////////////////
  coral::AttributeList result;
  result.extend("cid",typeid(int));
  result.extend("name",typeid(std::string));
  result.extend("type",typeid(std::string));
  result.extend("notnull",typeid(int));
  result.extend("dflt_value",typeid(int));
  result.extend("pk",typeid(int));
  std::string vinfo=std::string("PRAGMA table_info(\"")+m_description->name()+std::string("\")");
  if( !stmt.prepare(vinfo) ) {
    throw coral::SchemaException("sqlite","Could not retrieve the name of a column in a view","View::refreshDescription" );
  }
  while( stmt.fetchNext() ) {
    if(!stmt.defineOutput( result ) ) {
      throw coral::SchemaException("sqlite","Could not retrieve the name of a column in a view","View::refreshDescription" );
    }
    bool isNullable=true;
    const std::string& columnName=result[1].data<std::string>();
    //std::cout<<"columnName "<<columnName<<std::endl;
    const std::string& columnType=result[2].data<std::string>(); //sql type
    //std::cout<<"columnType "<<columnType<<std::endl;
    if( result[3].data<int>()==1 || result[3].data<int>()==99 ) {
      //std::cout<<"is not null"<<std::endl;
      isNullable=false;
    }
    if( result[5].data<int>()==1 ) {
      isNullable=false;
    }
    m_description->insertColumn( columnName, m_properties->typeConverter().cppTypeForSqlType(columnType) );
    if ( !isNullable ) {
      m_description->setNotNullConstraint( columnName );
    }
  }
}

void
coral::SQLiteAccess::View::refreshDefinition()
{

  std::ostringstream os;
  os<<"SELECT sql FROM sqlite_master WHERE type=\'view\' AND name=\'"<<m_description->name()<<"\'";
  coral::SQLiteAccess::SQLiteStatement stmt( m_properties );
  if ( !stmt.prepare(os.str()) ) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not read the definition of a view","View::refreshDefinition");
  }
  coral::AttributeList result;
  result.extend( "sql", typeid(std::string) );
  std::string sqlseg;
  while( stmt.fetchNext() ) {
    stmt.defineOutput( result );
    sqlseg =result["sql"].data<std::string>();
  }
  if(sqlseg.empty()) {
    throw coral::SchemaException( m_properties->domainProperties().service()->name(),"Could not read the definition of a view","View::refreshDefinition");
  }
  m_definitionString=sqlseg.substr(sqlseg.find("SELECT "));
  stmt.reset();
}
