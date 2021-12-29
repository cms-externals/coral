#include "TableQueryThread.h"
#include "TestEnv/TestingConfig.h"

#include <stdexcept>
#include <sstream>
#include <memory>
#include <cmath>

void TableQueryThread::operator()()
{
  try {
    //m_reporter.reportToSTDOUT( os1.str() );
    std::ostringstream osTableName;
    osTableName << m_table << "_" << m_tableNumber;
    const std::string tableName = osTableName.str();

    std::ostringstream os2;
    os2 << "TableCreateAndQueryThread no " << m_tableNumber << " about to perform a query";
    //m_reporter.reportToSTDOUT( os2.str() );

    std::unique_ptr<coral::IQuery> query( m_schema.tableHandle( tableName ).newQuery() );

    coral::AttributeList outputBuffer;
    outputBuffer.extend<double>( "RES" );

    query->addToOutputList( "F+D", "RES" );
    query->addToOrderList( "I" );
    query->defineOutput( outputBuffer );

    coral::ICursor& cursor = query->execute();
    int row02 = 0;
    double& res = outputBuffer[0].data<double>();
    while ( cursor.next() ) {
      if ( ::fabs( res - ( 2* row02 + 0.001001 * m_tableNumber ) ) > 0.00001 )
        throw std::runtime_error( "Unexpected data" );
      ++row02;
    }

    if ( row02 != 100 )
      throw std::runtime_error( "Unexpected number of rows" );

  }
  catch ( std::exception& e ) {
    std::ostringstream os;
    os << "[OVAL] : Standard C++ exception raised in TableCreateAndQueryThread no " << m_tableNumber << " : " << e.what();
    //m_reporter.reportToSTDERR( os.str() );
    throw;
  }
  catch (...) {
    std::ostringstream os;
    os << "[OVAL] : Unknonwn exception ... raised in TableCreateAndQueryThread no " << m_tableNumber;
    //m_reporter.reportToSTDERR( os.str() );
    throw;
  }

  std::ostringstream os;
  os << "TableCreateAndQueryThread no " << m_tableNumber << " ended gracefully";
  //m_reporter.reportToSTDOUT( os.str() );
}
