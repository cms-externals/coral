#include "ReadThread.h"
#include "Reporter.h"
#include "GlobalStatus.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include <sstream>
#include <memory>
#include <stdexcept>


void
ReadThread::operator()()
{
  std::ostringstream osi;
  osi << "Starting read thread " << m_id << "/" << m_tableId;
  m_reporter.reportToSTDOUT( osi.str() );

  try {
    std::ostringstream osTableName;
    osTableName << "T_" << m_id << "_" <<  m_tableId;
    std::string tableName;
    m_env->addTablePrefix(tableName, osTableName.str() );
    coral::ITable& table = m_schema.tableHandle( tableName );

    std::unique_ptr<coral::IQuery> query( table.newQuery() );

    coral::AttributeList rowBuffer;
    rowBuffer.extend<int>( "DATA" );
    int& data = rowBuffer[0].data<int>();

    query->addToOutputList( "DATA" );
    query->defineOutput( rowBuffer );

    coral::AttributeList whereData;
    whereData.extend<int>( "CUT" );
    int cut = m_id + m_tableId;
    whereData[0].data<int>() = cut;
    const std::string clause = "ID > :CUT";
    query->setCondition( clause, whereData );
    query->addToOrderList( "ID" );

    coral::ICursor& cursor = query->execute();
    int i = cut + 1;
    while ( cursor.next() ) {
      int expectedData = i + 1000 * m_tableId + 10000 * m_id;
      if ( data != expectedData ) {
        std::ostringstream osError;
        osError << "Unexpected data  for table " << m_tableId << ", thread " << m_id << " and row " << i - cut << " : " << data << " instead of " << expectedData;
        throw std::runtime_error( osError.str() );
      }
      ++i;
    }

    if ( i != 1000 )
      throw std::runtime_error( "Unexpected number of rows returned" );

  }
  catch ( std::exception& e ) {
    m_ok.setFalse();
    std::ostringstream os;
    os << "Exception caught in read thread " << m_id << "/" << m_tableId << " : " << e.what();
    m_reporter.reportToSTDERR( os.str() );
  }
  catch ( ... ) {
    m_ok.setFalse();
    std::ostringstream os;
    os << "Unknown exception caught in read thread " << m_id << "/" << m_tableId;
    m_reporter.reportToSTDERR( os.str() );
  }
  std::ostringstream osf;
  osf << "End of read thread " << m_id << "/" << m_tableId;
  m_reporter.reportToSTDOUT( osf.str() );
}
