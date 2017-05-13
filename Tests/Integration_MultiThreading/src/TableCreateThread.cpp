#include "TableCreateThread.h"
#include "TestEnv/TestingConfig.h"

#include <stdexcept>
#include <sstream>
#include <memory>
#include <cmath>
#include "CoralBase/MessageStream.h"

#define LOG( msg ){ coral::MessageStream myMsg("MTHREAD"); myMsg << coral::Always << msg << coral::MessageStream::endmsg; }

void TableCreateThread::operator()()
{
  try {
    LOG("Prepare table for table number " << m_tableNumber << " ... starting");
    // The table name
    std::ostringstream osTableName;
    osTableName << m_table << "_" << m_tableNumber;
    const std::string tableName = osTableName.str();

    LOG("Prepare table for table number " << m_tableNumber << " ... drop old table");
    m_schema.dropIfExistsTable( tableName );

    LOG("Prepare table for table number " << m_tableNumber << " ... schema");
    coral::TableDescription description;
    description.setName( tableName );
    description.insertColumn( "I", "int" );
    description.insertColumn( "F", "float" );
    description.insertColumn( "D", "double" );
    description.setPrimaryKey( "I" );
    description.setNotNullConstraint( "F" );

    coral::ITable& table = m_schema.createTable( description );
    table.privilegeManager().grantToPublic( coral::ITablePrivilegeManager::Select );

    LOG("Prepare table for table number " << m_tableNumber << " ... filling");

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
      f = row01 + (float)0.001 * m_tableNumber;
      d = row01 + 0.000001 * m_tableNumber;
      table.dataEditor().insertRow( rowBuffer );
    }
    LOG("Prepare table for table number " << m_tableNumber << " ... done");
  }
  catch ( std::exception& e ) {
    std::ostringstream os;
    os << "[OVAL] : Standard C++ exception raised in TableCreateAndQueryThread no " << m_tableNumber << " : " << e.what();
    //m_reporter.reportToSTDERR( os.str() );
  }
  catch (...) {
    std::ostringstream os;
    os << "[OVAL] : Unknonwn exception ... raised in TableCreateAndQueryThread no " << m_tableNumber;
    //m_reporter.reportToSTDERR( os.str() );
  }

  std::ostringstream os;
  os << "TableCreateAndQueryThread no " << m_tableNumber << " ended gracefully";
  //m_reporter.reportToSTDOUT( os.str() );
}
