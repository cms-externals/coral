#include "SchemaCopyApp.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/TableDescription.h"
#include "RelationalAccess/SchemaException.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/Blob.h"

#include <set>
#include <iostream>
#include <stdexcept>

SchemaCopyApp::SchemaCopyApp( const std::string& connectionString, const std::string& source, const std::string& dest, const std::string& userName, const std::string& password )
  : TestBase(), m_connectionString( connectionString ), m_source( source ), m_dest( dest), m_userName( userName ), m_password( password )
{
}

SchemaCopyApp::~SchemaCopyApp()
{
  if( this->m_session != 0 )
  {
    delete this->m_session;
    this->m_session = 0;
  }
}

void SchemaCopyApp::readDbObjects()
{
  m_session = this->connect( m_connectionString + "/" + m_source, m_userName, m_password, coral::ReadOnly );

  m_session->transaction().start();

  coral::ISchema& schema = m_session->nominalSchema();

  // Read the source schema objects
  std::set<std::string> tableList = schema.listTables();

  if( tableList.empty() )
  {
    std::cout << "Source schema " << m_connectionString << "/" << m_source << " has no database objects..." << std::endl;
    return;
  }

  for( std::set<std::string>::iterator sit = tableList.begin(); sit != tableList.end(); ++sit )
  {
    m_tables.push_back( TabDesc( (*sit) , schema.tableHandle(*sit).description() ) );

    std::cout << "Reading \"" << schema.tableHandle(*sit).description().name() << "\"(" << (*sit) << ") table description from schema " << m_source << std::endl;
  }

  m_session->transaction().commit();
}

void SchemaCopyApp::cpDbObjects()
{
  coral::ISession* session = this->connect( m_connectionString + "/" + m_dest, m_userName, m_password, coral::Update );

  session->transaction().start();

  try
  {
    coral::ISchema& schema = session->nominalSchema();

    for( std::list<TabDesc>::iterator sit = m_tables.begin(); sit != m_tables.end(); ++sit )
    {
      std::cout << "Copying table database object \""  << (*sit).m_desc.name() << "\" to schema \"" << m_dest << std::endl;

      const coral::ITableDescription& tdesc = (*sit).m_desc;

      // List of all indices read from the source
      std::cout << "Table " << tdesc.name() << " has the fillowing indices: " << std::endl;
      int numIdx = tdesc.numberOfIndices();
      for( int i = 0; i < numIdx; i++ )
      {
        std::cout << tdesc.index(i).name() << std::endl;
      }

      schema.createTable( tdesc );

      std::cout << "Table " << tdesc.name() << " created................................" << std::endl;
    }

    session->transaction().commit();

    session->transaction().start();

    std::set<std::string> tableList = schema.listTables();

    if( tableList.size() != m_tables.size() )
    {
      session->transaction().rollback();
      throw coral::SchemaException( "Could not copy table database objects from schema " + m_source + " to schema " + m_dest, "SchemaCopyApp", "copyDbObjects" );
    }

    session->transaction().commit();
  }
  catch(const std::exception& e )
  {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    throw;
  }

  delete session;
}

void SchemaCopyApp::run()
{
  this->readDbObjects();

  this->cpDbObjects();
}
