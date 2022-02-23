// Include files
#include <limits>
#include <map>
#include <stdexcept>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/../src/isNaN.h"
#include "CoralBase/../tests/Common/CoralCppUnitDBTest.h"
#include "CoralCommon/Utilities.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/TableDescription.h"

// Local (COOL) include files
#include "types.h"
#include "StorageType.h"

// Workaround for CORAL bug #43972 (ORA-24801 while filling LOB locator)
#define NOBLOB 1

// Skip NaNs on MySQL?
#define skipNanMySQL true

//-----------------------------------------------------------------------------

using namespace cool;

namespace coral
{

  // Skip second creation of Oracle table for Frontier if possible
  static bool s_filledOracleTable = false;

  // Type definition (simpler replacement of cool::FieldSpecification class)
  typedef std::pair<std::string, cool::StorageType::TypeId> FieldSpecification;

  // Type definition (simpler replacement of cool::RecordSpecification class)
  typedef std::vector<FieldSpecification> RecordSpecification;

  /** @class CoralReferenceDBTest
   *
   *  Comprehensive test for CORAL storage and retrieval of several
   *  distinct values of all persistent data types supported by COOL.
   *
   *  @author Andrea Valassi
   *  @date   2007-02-15
   */

  class CoralReferenceDBTest : public CoralCppUnitDBTest
  {

    CPPUNIT_TEST_SUITE( CoralReferenceDBTest );
    CPPUNIT_TEST( test_oneBackend );
    CPPUNIT_TEST_SUITE_END();

  public:

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    static void insertColumn( coral::TableDescription& desc,
                              const std::string& name,
                              const cool::StorageType::TypeId& typeId )
    {
      const cool::StorageType& type = cool::StorageType::storageType( typeId );
      const std::string typeName =
        coral::AttributeSpecification::typeNameForId( type.cppType() );
      int maxSize = type.maxSize();
      const bool fixedSize = false;
      // Workaround for CORAL bug #22543 - START
      // Bug is MySQL: BLOB(nnnnn) used instead of BLOB (and/or MEDIUMBLOB...)
      // Quick workaround applies to all but should not harm
      if ( type == StorageType::Blob64k ) maxSize=0;
      // Workaround for CORAL bug #22543 - END
      desc.insertColumn( name, typeName, maxSize, fixedSize );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    static void formatAttributeList( coral::AttributeList& data,
                                     const RecordSpecification& spec )
    {
      for ( RecordSpecification::const_iterator
              it = spec.begin(); it != spec.end(); it++ )
      {
        data.extend( it->first,
                     StorageType::storageType(it->second).cppType() );
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    static const RecordSpecification& referenceRecordSpec()
    {
      static RecordSpecification spec;
      if ( spec.size() == 0 )
      {
        // Payload metadata - unique ID and unique description
        spec.push_back( FieldSpecification
                        ( "ID", cool::StorageType::Int32 ) );
        spec.push_back( FieldSpecification
                        ( "Desc", cool::StorageType::String255 ) );
        // Payload data
        spec.push_back( FieldSpecification
                        ( "A_BOOL", cool::StorageType::Bool ) );
        spec.push_back( FieldSpecification
                        ( "A_UCHAR", cool::StorageType::UChar ) );
        spec.push_back( FieldSpecification
                        ( "A_INT16", cool::StorageType::Int16 ) );
        spec.push_back( FieldSpecification
                        ( "A_UINT16", cool::StorageType::UInt16 ) );
        spec.push_back( FieldSpecification
                        ( "A_INT32", cool::StorageType::Int32 ) );
        spec.push_back( FieldSpecification
                        ( "A_UINT32", cool::StorageType::UInt32 ) );
        spec.push_back( FieldSpecification
                        ( "A_UINT63", cool::StorageType::UInt63 ) );
        spec.push_back( FieldSpecification
                        ( "A_INT64", cool::StorageType::Int64 ) );
        spec.push_back( FieldSpecification
                        ( "A_FLOAT", cool::StorageType::Float ) );
        spec.push_back( FieldSpecification
                        ( "A_DOUBLE", cool::StorageType::Double ) );
        spec.push_back( FieldSpecification
                        ( "A_STRING255", cool::StorageType::String255 ) );
        spec.push_back( FieldSpecification
                        ( "A_STRING4K", cool::StorageType::String4k ) );
        spec.push_back( FieldSpecification
                        ( "A_STRING64K", cool::StorageType::String64k ) );
        spec.push_back( FieldSpecification
                        ( "A_STRING16M", cool::StorageType::String16M ) );
#ifndef NOBLOB
        spec.push_back( FieldSpecification
                        ( "A_BLOB64K", cool::StorageType::Blob64k ) );
        spec.push_back( FieldSpecification
                        ( "A_BLOB16M", cool::StorageType::Blob16M ) );
#endif
      }
      return spec;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    static const coral::TableDescription& referenceTableDescription()
    {
      static coral::TableDescription desc;
      if ( desc.name() == "" )
      {
        desc.setName( BuildUniqueTableName( "CORALREF_COOL" ) );
        // Insert all fields from the reference record specification
        const RecordSpecification& spec = referenceRecordSpec();
        for ( RecordSpecification::const_iterator
                it = spec.begin(); it != spec.end(); it++ )
          insertColumn( desc, it->first, it->second );
        // Payload metadata - unique ID and unique description
        desc.setPrimaryKey( "ID" );
        desc.setUniqueConstraint( "Desc" );
      }
      return desc;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    static const coral::AttributeList& referenceData( int index, bool skipNaN )
    {
      static std::map<int,coral::AttributeList> dataMap_withNaN;
      static std::map<int,coral::AttributeList> dataMap_skipNaN;
      std::map<int,coral::AttributeList>& dataMap = dataMap_withNaN;
      if ( skipNaN ) dataMap = dataMap_skipNaN;
      if ( dataMap.find( index ) == dataMap.end() )
      {
        // ******************************************************
        // *** Index = -4       *** NaN float/double value    ***
        // *** Index = -3       *** null value                ***
        // *** Index = -2       *** min value                 ***
        // *** Index = -1       *** max value                 ***
        // *** Index in [0,255] *** test all characters/bytes ***
        // ******************************************************
        if ( ( index>=-4 && index<=255) )
        {
          dataMap[index] = coral::AttributeList();
          coral::AttributeList& data = dataMap[index];
          const RecordSpecification& spec = referenceRecordSpec();
          formatAttributeList( data, spec );
          data["ID"].setNull( false );
          data["ID"].setValue( (cool::Int32)index );
          data["Desc"].setNull( false );
          data["A_BOOL"].setNull( false );
          data["A_UCHAR"].setNull( false );
          data["A_INT16"].setNull( false );
          data["A_UINT16"].setNull( false );
          data["A_INT32"].setNull( false );
          data["A_UINT32"].setNull( false );
          data["A_UINT63"].setNull( false );
          data["A_INT64"].setNull( false );
          data["A_FLOAT"].setNull( false );
          data["A_DOUBLE"].setNull( false );
          data["A_STRING255"].setNull( false );
          data["A_STRING4K"].setNull( false );
          data["A_STRING64K"].setNull( false );
          data["A_STRING16M"].setNull( false );
#ifndef NOBLOB
          data["A_BLOB64K"].setNull( false );
          data["A_BLOB16M"].setNull( false );
#endif
          // ******************************************************
          // *** Index = -4       *** NaN float/double value    ***
          // ******************************************************
          if ( index == -4 )
          {
            data["Desc"].setValue( std::string("NaN float/double") );
            data["A_BOOL"].setValue( false );
            data["A_UCHAR"].setValue( (unsigned char)'\0' );
            data["A_INT16"].setValue( (cool::Int16)0 );
            data["A_UINT16"].setValue( (cool::UInt16)0 );
            data["A_INT32"].setValue( (cool::Int32)0 );
            data["A_UINT32"].setValue( (cool::UInt32)0 );
            data["A_UINT63"].setValue( (cool::UInt63)0 );
            data["A_INT64"].setValue( (cool::Int64)0 );
            if ( !skipNaN )
            {
              data["A_FLOAT"].setValue( std::numeric_limits<float>::quiet_NaN() );
              data["A_DOUBLE"].setValue( std::numeric_limits<double>::quiet_NaN() );
            }
            else
            {
              data["A_FLOAT"].setValue( (cool::Float)0 );
              data["A_DOUBLE"].setValue( (cool::Double)0 );
            }
            data["A_STRING255"].setValue( std::string("DUMMY") );
            data["A_STRING4K"].setValue( std::string("DUMMY") );
            data["A_STRING64K"].setValue( std::string("DUMMY") );
            data["A_STRING16M"].setValue( std::string("DUMMY") );
#ifndef NOBLOB
            data["A_BLOB64K"].data<coral::Blob>().resize(0);
            data["A_BLOB16M"].data<coral::Blob>().resize(0);
#endif
          }
          // ******************************************************
          // *** Index = -3       *** null value                ***
          // ******************************************************
          else if ( index == -3 )
          {
            data["Desc"].setValue( std::string("NULL values") );
            data["A_BOOL"].setNull( true );
            data["A_UCHAR"].setNull( true );
            data["A_INT16"].setNull( true );
            data["A_UINT16"].setNull( true );
            data["A_INT32"].setNull( true );
            data["A_UINT32"].setNull( true );
            data["A_UINT63"].setNull( true );
            data["A_INT64"].setNull( true );
            data["A_FLOAT"].setNull( true );
            data["A_DOUBLE"].setNull( true );
            data["A_STRING255"].setNull( true );
            data["A_STRING4K"].setNull( true );
            data["A_STRING64K"].setNull( true );
            data["A_STRING16M"].setNull( true );
#ifndef NOBLOB
            data["A_BLOB64K"].setNull( true );
            data["A_BLOB16M"].setNull( true );
#endif
          }
          // ******************************************************
          // *** Index = -2       *** min value                 ***
          // ******************************************************
          else if ( index == -2 )
          {
            data["Desc"].setValue( std::string("min values") );
            data["A_BOOL"].setValue( false );
            data["A_UCHAR"].setValue( cool::UCharMin );
            data["A_INT16"].setValue( cool::Int16Min );
            data["A_UINT16"].setValue( cool::UInt16Min );
            data["A_INT32"].setValue( cool::Int32Min );
            data["A_UINT32"].setValue( cool::UInt32Min );
            data["A_UINT63"].setValue( cool::UInt63Min );
            data["A_INT64"].setValue( cool::Int64Min );
            data["A_FLOAT"].setValue( (cool::Float)(0.123456789012345678901234567890) );
            data["A_DOUBLE"].setValue( (cool::Double)(0.123456789012345678901234567890) );
            data["A_STRING255"].setValue( std::string("") );
            data["A_STRING4K"].setValue( std::string("") );
            data["A_STRING64K"].setValue( std::string("") );
            data["A_STRING16M"].setValue( std::string("") );
#ifndef NOBLOB
            data["A_BLOB64K"].data<coral::Blob>().resize(0);
            data["A_BLOB16M"].data<coral::Blob>().resize(0);
#endif
          }
          // ******************************************************
          // *** Index = -1       *** max value                 ***
          // ******************************************************
          else if ( index == -1 )
          {
            data["Desc"].setValue( std::string("MAX VALUES") );
            data["A_BOOL"].setValue( true );
            data["A_UCHAR"].setValue( cool::UCharMax );
            data["A_INT16"].setValue( cool::Int16Max );
            data["A_UINT16"].setValue( cool::UInt16Max );
            data["A_INT32"].setValue( cool::Int32Max );
            data["A_UINT32"].setValue( cool::UInt32Max );
            data["A_UINT63"].setValue( cool::UInt63Max );
            data["A_INT64"].setValue( cool::Int64Max );
            data["A_FLOAT"].setValue( (cool::Float)(0.987654321098765432109876543210) );
            data["A_DOUBLE"].setValue( (cool::Double)(0.987654321098765432109876543210) );
            std::string high = "HIGH";
            data["A_STRING255"].setValue( high );
            data["A_STRING4K"].setValue( high );
            data["A_STRING64K"].setValue( high );
            data["A_STRING16M"].setValue( high );
#ifndef NOBLOB
            data["A_BLOB64K"].data<coral::Blob>().resize(0);
            data["A_BLOB16M"].data<coral::Blob>().resize(0);
#endif
          }
          // ******************************************************
          // *** Index in [0,255] *** test all characters/bytes ***
          // ******************************************************
          else
          {
            std::stringstream sIndex;
            sIndex << index;
            data["Desc"].setValue( "Value #"+sIndex.str() );
            data["A_BOOL"].setValue( true );
            unsigned char uc = index;
            data["A_UCHAR"].setValue( uc );
            data["A_INT16"].setValue( (cool::Int16)(-index) );
            data["A_UINT16"].setValue( (cool::UInt16)(index) );
            data["A_INT32"].setValue( (cool::Int32)(-index) );
            data["A_UINT32"].setValue( (cool::UInt32)(index) );
            data["A_UINT63"].setValue( (cool::UInt63)(index) );
            data["A_INT64"].setValue( (cool::Int64)(-index) );
            data["A_FLOAT"].setValue( (cool::Float)(-index) );
            data["A_DOUBLE"].setValue( (cool::Double)(index) );
            std::string sUc = std::string( 1, (char)uc );
            data["A_STRING255"].setValue( sUc );
            data["A_STRING4K"].setValue( sUc );
            data["A_STRING64K"].setValue( sUc );
            data["A_STRING16M"].setValue( sUc );
#ifndef NOBLOB
            data["A_BLOB64K"].data<coral::Blob>().resize
              ( sizeof(unsigned char) );
            unsigned char* pUc;
            pUc = static_cast<unsigned char*>
              ( data["A_BLOB64K"].data<coral::Blob>().startingAddress() );
            *pUc = uc;
            data["A_BLOB16M"].data<coral::Blob>().resize
              ( sizeof(unsigned char) );
            pUc = static_cast<unsigned char*>
              ( data["A_BLOB16M"].data<coral::Blob>().startingAddress() );
            *pUc = uc;
#endif
          }
        }
        // Unknown index: throw an exception
        else
        {
          std::stringstream msg;
          msg << "Unknown index value for reference data: " << index;
          throw std::runtime_error( msg.str() );
        }
      }
      return dataMap[index];
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void fillReferenceData( const std::string& connectString )
    {
      if ( connectString == BuildUrl( "Oracle", false )
           && s_filledOracleTable )
      {
        std::cout << std::endl
                  << "Oracle reference table already exists" << std::endl;
      }
      std::cout << std::endl
                << "Create and fill reference table..." << std::endl;
      std::cout << "Reference table: "
                << referenceTableDescription().name() << std::endl;
      coral::IConnectionService& connSvc = connectionService();
      coral::AccessMode accessMode = coral::Update;
      std::unique_ptr<coral::ISessionProxy>
        session( connSvc.connect( connectString, accessMode ) );
      session->transaction().start( false ); // read-write
      // ** START ** write reference data
      const coral::TableDescription& desc = referenceTableDescription();
      session->nominalSchema().dropIfExistsTable( desc.name() );
      coral::ITable& table = session->nominalSchema().createTable( desc );
      for ( int index =-4; index<=255; index++ )
      {
        bool skipNaN = ( skipNanMySQL &&
                         connectString == BuildUrl( "MySQL", false ) );
        table.dataEditor().insertRow( referenceData( index, skipNaN ) );
      }
      if ( connectString == BuildUrl( "Oracle", false ) )
      {
        s_filledOracleTable = true;
        table.privilegeManager().grantToUser
          ( "PUBLIC", coral::ITablePrivilegeManager::Select );
      }
      // **  END  ** write reference data
      session->transaction().commit();
      std::cout << "Create and fill reference table... DONE" << std::endl;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    template<class T>
    void assertEqualMessage( const std::string& msg,
                             const T& refValue,
                             const T& value )
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( msg, refValue, value );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    template<class T>
    void testPayload( const std::string& sIndex,
                      const std::string& name,
                      const coral::AttributeList& refRow,
                      const coral::AttributeList& row,
                      const bool isNull,
                      const std::string& connectString )
    {
      // Compare nullness
      // Workaround for 'bug #22381' (Oracle feature: it treats '' as NULL)
      if ( ( connectString == BuildUrl( "Oracle", true ) ||
             connectString == BuildUrl( "Frontier", true ) ||
             connectString == BuildUrl( "CoralServer-Oracle", true ) ) &&
           ( sIndex == "Index#-2 " || sIndex == "Index#0 " ) &&
           typeid( T ) == typeid( std::string ) )
      {
        std::cout << "*** WARNING!! *** Oracle '' == NULL " << std::endl;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex + name + " isNullOracle''",
                                      true,
                                      row[ name ].isNull() );
      }
      // Compare nullness
      // Workaround for 'bug #72147' (SQLite feature: it stores NaN as NULL)
      else if ( connectString == BuildUrl( "SQLite", true ) &&
                ( sIndex == "Index#-4 " ) &&
                ( typeid( T ) == typeid( float ) ||
                  typeid( T ) == typeid( double ) ) )
      {
        std::cout << "*** WARNING!! *** SQLite NaN == NULL " << std::endl;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex + name + " isNullSQLiteNaN",
                                      true,
                                      row[ name ].isNull() );
      }
      // Compare nullness for all other cases
      else
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex + name + " isNull1",
                                      refRow[ name ].isNull(),
                                      row[ name ].isNull() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex + name + " isNull2",
                                      isNull,
                                      row[ name ].isNull() );
      }
      // Compare values (only if not null)
      if ( !row[ name ].isNull() )
      {
        /*
        std::cout << sIndex << name
                  << " RefValue='" << refRow[ name ].data<T>() << "'"
                  << " Value='" << row[ name ].data<T>() << "'"
                  << std::endl;
        if ( typeid( T ) == typeid( std::string ) )
          std::cout << sIndex << name
                    << " RefSize=" << refRow[ name ].data<std::string>().size()
                    << " Size=" << row[ name ].data<std::string>().size()
                    << std::endl;
        */
        // WARNING: skip '\0' string for all four backends
        if ( sIndex == "Index#0 " && typeid( T ) == typeid( std::string ) )
        {
          std::cout << "*** WARNING!! *** Skip test for "
                    << sIndex << name << std::endl;
        }
        else
        {
          assertEqualMessage( sIndex + name + " data",
                              refRow[ name ].data<T>(),
                              row[ name ].data<T>() );
        }
      }
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void readReferenceData( const std::string& connectString )
    {
      std::cout << "Read from reference table..." << std::endl;
      coral::IConnectionService& connSvc = connectionService();
      coral::AccessMode accessMode = coral::ReadOnly;
      std::unique_ptr<coral::ISessionProxy>
        session( connSvc.connect( connectString, accessMode ) );
      session->transaction().start( true ); // read-only
      // Dump the supported C++ and SQL types for this backend
      // (except for CoralAccess where TypeConverter is not yet supported)
      if ( connectString != BuildUrl( "CoralServer-Oracle", true ) )
      {
        std::cout << std::endl << "Supported C++ types: " << std::endl;
        std::set<std::string> cppTypes =
          session->typeConverter().supportedCppTypes();
        for ( std::set<std::string>::const_iterator iType = cppTypes.begin();
              iType != cppTypes.end();
              ++iType )
        {
          std::cout << *iType << " -> "
                    << session->typeConverter().sqlTypeForCppType( *iType )
                    << std::endl;
        }
        std::cout << std::endl << "Supported SQL types: " << std::endl;
        std::set<std::string> sqlTypes =
          session->typeConverter().supportedSqlTypes();
        for ( std::set<std::string>::const_iterator iType = sqlTypes.begin();
              iType != sqlTypes.end();
              ++iType )
        {
          std::cout << *iType << " -> "
                    << session->typeConverter().cppTypeForSqlType( *iType )
                    << std::endl;
        }
        std::cout << std::endl;
      }
      // ** START ** read reference data
      coral::ISchema& schema = session->nominalSchema();
      std::unique_ptr<coral::IQuery> query( schema.newQuery() );
      query->addToTableList( referenceTableDescription().name() );
      query->addToOrderList( "ID" );
      coral::AttributeList dataBuffer;
      const RecordSpecification& spec = referenceRecordSpec();
      formatAttributeList( dataBuffer, spec );
      query->defineOutput( dataBuffer );
      coral::ICursor& cursor = query->execute();
      std::vector< coral::AttributeList > rows;
      while ( cursor.next() ) rows.push_back( cursor.currentRow() );
      CPPUNIT_ASSERT_EQUAL_MESSAGE( "# rows", 260, (int)rows.size() );
      for ( int iRow = 0; iRow<=259; iRow++ )
      {
        int index = iRow-4; // iRow in [0,259], index in [-4,255]
        if ( index <= 2 || index >= 253 )
          std::cout << "Read from reference table (" << index << ")"
                    << std::endl;
        else if ( index == 3 )
          std::cout << "Read from reference table (3...252)"
                    << std::endl;
        std::stringstream ssIndex;
        ssIndex << "Index#" << index << " ";
        std::string sIndex = ssIndex.str();
        const coral::AttributeList row = rows[iRow];
        bool skipNaN =
          ( skipNanMySQL &&
            ( connectString == BuildUrl( "MySQL", true ) ||
              connectString == BuildUrl( "CoralServer-MySQL", true ) ) );
        const coral::AttributeList& refRow = referenceData(index, skipNaN);

        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"ID isNull1",
                                      refRow["ID"].isNull(),
                                      row["ID"].isNull() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"ID isNull2",
                                      false,
                                      row["ID"].isNull() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"ID data",
                                      refRow["ID"].data<cool::Int32>(),
                                      row["ID"].data<cool::Int32>() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"Desc isNull1",
                                      refRow["Desc"].isNull(),
                                      row["Desc"].isNull() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"Desc isNull2",
                                      false,
                                      row["Desc"].isNull() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( sIndex+"Desc data",
                                      refRow["Desc"].data<std::string>(),
                                      row["Desc"].data<std::string>() );

        bool isNull = false;
        if ( index == -3 ) isNull = true;

        testPayload<cool::Bool>
          ( sIndex, "A_BOOL", refRow, row, isNull, connectString );
        testPayload<cool::UChar>
          ( sIndex, "A_UCHAR", refRow, row, isNull, connectString );
        testPayload<cool::Int16>
          ( sIndex, "A_INT16", refRow, row, isNull, connectString );
        testPayload<cool::UInt16>
          ( sIndex, "A_UINT16", refRow, row, isNull, connectString );
        testPayload<cool::Int32>
          ( sIndex, "A_INT32", refRow, row, isNull, connectString );
        testPayload<cool::UInt32>
          ( sIndex, "A_UINT32", refRow, row, isNull, connectString );
        //testPayload<cool::UInt63>
        //  ( sIndex, "A_UINT63", refRow, row, isNull, connectString );
        //testPayload<cool::Int64>
        //  ( sIndex, "A_INT64", refRow, row, isNull, connectString );
        testPayload<cool::Float>
          ( sIndex, "A_FLOAT", refRow, row, isNull, connectString );
        testPayload<cool::Double>
          ( sIndex, "A_DOUBLE", refRow, row, isNull, connectString );
        testPayload<cool::String255>
          ( sIndex, "A_STRING255", refRow, row, isNull, connectString );
        testPayload<cool::String4k>
          ( sIndex, "A_STRING4K", refRow, row, isNull,connectString );
        testPayload<cool::String64k>
          ( sIndex, "A_STRING64K", refRow, row, isNull, connectString );
        testPayload<cool::String16M>
          ( sIndex, "A_STRING16M", refRow, row, isNull, connectString );
#ifndef NOBLOB
        // TO DO: BLOB templated method
        //testPayload<cool::Blob64k>
        //  ( sIndex, "A_BLOB64K", refRow, row, isNull );
        //testPayload<cool::Blob16M>
        //  ( sIndex, "A_BLOB16M", refRow, row, isNull );
#endif
      }
      // **  END  ** read reference data
      session->transaction().commit();
      std::cout << "Read from reference table... DONE" << std::endl;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    void test_oneBackend()
    {
      const std::string connectStringW = UrlRW();
      const std::string connectStringR = UrlRO();
      fillReferenceData( connectStringW );
      if( connectStringW == BuildUrl( "Oracle", false ) )
        coral::sleepSeconds(1);  // Workaround for ORA-01466
      readReferenceData( connectStringR );
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    coral::IConnectionService& connectionService()
    {
      static coral::ConnectionService connSvc;
      return connSvc;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    CoralReferenceDBTest(){}

    ~CoralReferenceDBTest(){}

    void setUp(){}

    void tearDown(){}

  };

  //---------------------------------------------------------------------------

  template<> void
  CoralReferenceDBTest::assertEqualMessage<float>( const std::string& msg,
                                                   const float& refValue,
                                                   const float& value )
  {
    if ( isNaN( refValue ) && isNaN( refValue ) ) return;  // Both are NaN: OK
    double exp = pow( (double)10, 5 );
    try
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( msg,
                                    floor( refValue * exp ) / exp,
                                    floor( value    * exp ) / exp );
    }
    catch(...)
    {
      std::streamsize prec = std::cout.precision();
      std::cout.precision(20);
      std::cout << msg << " RefValue='" << refValue << "'"
                << " Value='" << value << "'" << std::endl;
      std::cout.precision(prec);
      throw;
    }
  }

  //---------------------------------------------------------------------------

  template<> void
  CoralReferenceDBTest::assertEqualMessage<double>( const std::string& msg,
                                                    const double& refValue,
                                                    const double& value )
  {
    if ( isNaN( refValue ) && isNaN( refValue ) ) return;  // Both are NaN: OK
    double exp = pow( (double)10, 14 );
    try
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE( msg,
                                    floor( refValue * exp ) / exp,
                                    floor( value    * exp ) / exp );
    }
    catch(...)
    {
      std::streamsize prec = std::cout.precision();
      std::cout.precision(20);
      std::cout << msg << " RefValue='" << refValue << "'"
                << " Value='" << value << "'" << std::endl;
      std::cout.precision(prec);
      throw;
    }
  }

  //---------------------------------------------------------------------------

  CPPUNIT_TEST_SUITE_REGISTRATION( CoralReferenceDBTest );

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

CORALCPPUNITTEST_MAIN( CoralReferenceDBTest )
