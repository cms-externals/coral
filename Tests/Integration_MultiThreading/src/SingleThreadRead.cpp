#include "SingleThreadRead.h"
#include "TestEnv/TestingConfig.h"

//-----------------------------------------------------------------------------

SingleThreadRead::SingleThreadRead( const TestEnv& env, size_t no )
  : Testing(env)
  , m_tableno( no )
{
}

//-----------------------------------------------------------------------------

SingleThreadRead::~SingleThreadRead()
{
}

//-----------------------------------------------------------------------------

void
SingleThreadRead::operator()()
{
  coral::ISessionProxy* session = newSession(0, coral::ReadOnly, false);

  session->transaction().start( true );

  coral::ISchema& schema = session->nominalSchema();

  std::ostringstream osTableName;
  osTableName << T2 << "_" << m_tableno;
  const std::string tableName = osTableName.str();

  std::unique_ptr<coral::IQuery> query( schema.tableHandle( tableName ).newQuery() );

  coral::AttributeList outputBuffer;
  outputBuffer.extend<double>( "RES" );

  query->addToOutputList( "F+D", "RES" );
  query->addToOrderList( "I" );
  query->defineOutput( outputBuffer );

  coral::ICursor& cursor = query->execute();
  int row02 = 0;
  double& res = outputBuffer[0].data<double>();
  while ( cursor.next() ) {
    if ( ::fabs( res - ( 2* row02 + 0.001001 * m_tableno ) ) > 0.00001 )
      throw std::runtime_error( "Unexpected data" );
    ++row02;
  }

  if ( row02 != 100 )
    throw std::runtime_error( "Unexpected number of rows" );


  session->transaction().commit();

  delete session;
}
