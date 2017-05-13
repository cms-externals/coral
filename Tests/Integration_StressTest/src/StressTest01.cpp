#include "StressTest01.h"
#include "TestEnv/TestingConfig.h"

//-----------------------------------------------------------------------------

StressTest01::StressTest01(const TestEnv& env, size_t blobsize)
  : Testing(env)
  , m_blobsize( blobsize )
{

}

//-----------------------------------------------------------------------------

void
StressTest01::prepare()
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start();

  coral::ISchema& schema = session.nominalSchema();

  schema.dropIfExistsTable(T1);

  coral::TableDescription description01( "SchemaDefinition_Test" );
  description01.setName(T1);

  description01.insertColumn( "id", coral::AttributeSpecification::typeNameForId( typeid(long long) ) );
  description01.insertColumn( "data", coral::AttributeSpecification::typeNameForId( typeid(coral::Blob) ) );

  description01.setPrimaryKey( "id" );

  coral::ITable& table01 = session.nominalSchema().createTable( description01 );
  // Fill the table with some content

  coral::AttributeList rowbuffer01;
  table01.dataEditor().rowBuffer( rowbuffer01 );

  for(size_t i = 1; i < 21; i++ )
  {
    rowbuffer01[0].data<long long>() = i;
    //fill a blob with random data
    fillBlob(rowbuffer01[1].data<coral::Blob>(), m_blobsize);

    table01.dataEditor().insertRow( rowbuffer01 );
  }

  session.transaction().commit();

  // Oracle feature
  coral::sleepSeconds( 1 );
}

//-----------------------------------------------------------------------------

void
StressTest01::run(size_t runs)
{
  coral::ISessionProxy& session = getSession();

  session.transaction().start(true);

  coral::ISchema& schema = session.nominalSchema();
  coral::ITable& table = schema.tableHandle(T1);

  coral::AttributeList mylist;
  mylist.extend( "id", typeid(long long));
  mylist.extend( "data", typeid(coral::Blob));

  for(size_t i = 0; i < runs; i++ )
  {
    coral::IQuery * query = table.newQuery();

    query->addToOutputList( "id" );
    query->addToOutputList( "data" );

    coral::ICursor& cursor = query->execute();
    while(cursor.next())
    {
      const coral::AttributeList& row = cursor.currentRow();

      std::cout << "[" << row[0].data<long long>() << "]";
      std::cout.flush();
    }
    std::cout << std::endl;

    delete query;
  }


  session.transaction().commit();
}

//-----------------------------------------------------------------------------
