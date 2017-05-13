#include "SingleThreadWrite.h"
#include "TestEnv/TestingConfig.h"

//-----------------------------------------------------------------------------

SingleThreadWrite::SingleThreadWrite( const TestEnv& env, size_t no )
  : Testing(env)
  , m_tableno( no )
{
}

//-----------------------------------------------------------------------------

SingleThreadWrite::~SingleThreadWrite()
{
}

//-----------------------------------------------------------------------------

void
SingleThreadWrite::operator()()
{
  coral::ISessionProxy* session = newSession(0, coral::Update, false);

  session->transaction().start();

  coral::ISchema& schema = session->nominalSchema();

  std::ostringstream osTableName;
  osTableName << T2 << "_" << m_tableno;
  const std::string tableName = osTableName.str();

  schema.dropIfExistsTable( tableName );

  coral::TableDescription description;
  description.setName( tableName );
  description.insertColumn( "I", "int" );
  description.insertColumn( "F", "float" );
  description.insertColumn( "D", "double" );
  description.setPrimaryKey( "I" );
  description.setNotNullConstraint( "F" );

  coral::ITable& table = schema.createTable( description );
  table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

  coral::AttributeList rowBuffer;
  rowBuffer.extend<int>( "I" );
  rowBuffer.extend<float>( "F" );
  rowBuffer.extend<double>( "D" );

  int& i = rowBuffer[0].data<int>();
  float& f = rowBuffer[1].data<float>();
  double& d = rowBuffer[2].data<double>();

  for ( int row01 = 0; row01 < 100; ++row01 )
  {
    i = row01;
    f = row01 + (float)0.001 * m_tableno;
    d = row01 + 0.000001 * m_tableno;
    table.dataEditor().insertRow( rowBuffer );
  }

  coral::sleepSeconds( 1 );

  session->transaction().commit();

  delete session;
}
