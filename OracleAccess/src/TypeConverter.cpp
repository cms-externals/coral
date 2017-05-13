#include <sstream>

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "CoralKernel/Service.h"
#include "RelationalAccess/SessionException.h"

#include "DomainProperties.h"
#include "TypeConverter.h"

coral::OracleAccess::TypeConverter::TypeConverter( const coral::OracleAccess::DomainProperties& properties ) :
  m_properties( properties ),
  m_supportedCppTypes(),
  m_supportedSqlTypes(),
  m_defaultCppToSqlMapping(),
  m_defaultSqlToCppMapping(),
  m_currentCppToSqlMapping(),
  m_currentSqlToCppMapping()
{
  //std::cout << "Create TypeConverter " << this << std::endl; // debug bug #80065
}


coral::OracleAccess::TypeConverter::~TypeConverter()
{
  //std::cout << "Delete TypeConverter " << this << std::endl; // debug bug #80065
}


void
coral::OracleAccess::TypeConverter::reset( int majorVersion )
{
  if ( majorVersion == 0 ) return;

  // Most recent supported version is Oracle 11g (see bug #86406)
  if ( majorVersion != 9 && majorVersion != 10 && majorVersion != 11 )
  {
    majorVersion = 11;
    static bool first = true;
    if ( first )
    {
      first = false;
      coral::MessageStream log( m_properties.service()->name() );
      log << coral::Warning
          << "Type conversion for Oracle server version " << majorVersion
          << " will assume the SQL types for Oracle server version 11"
          << coral::MessageStream::endmsg;
    }
  }

  m_supportedCppTypes.clear();
  m_supportedSqlTypes.clear();
  m_defaultCppToSqlMapping.clear();
  m_defaultSqlToCppMapping.clear();
  m_currentCppToSqlMapping.clear();
  m_currentSqlToCppMapping.clear();

  // Define here the default C++ -> SQL mappings (Whatever the AttributeList supports...).
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(bool)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(bool)),
                                                   "NUMBER(1)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(char)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(char)),
                                                   "CHAR(1)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned char)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned char)),
                                                   "NUMBER(3)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(short)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(short)),
                                                   "NUMBER(5)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned short)),
                                                   "NUMBER(5)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(int)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(int)),
                                                   "NUMBER(10)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned int)),
                                                   "NUMBER(10)" ) );

  std::string longRepresentation = "NUMBER(20)";
  if ( sizeof(long) < sizeof(long long) ) longRepresentation = "NUMBER(20)";

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long)),
                                                   longRepresentation ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)),
                                                   longRepresentation ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long long)),
                                                   "NUMBER(20)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)),
                                                   "NUMBER(20)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(float)) );
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(double)) );
  if ( majorVersion > 9 ) {
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(float)),
                                                     "BINARY_FLOAT" ) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(double)),
                                                     "BINARY_DOUBLE" ) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long double)),
                                                     "BINARY_DOUBLE" ) );
  }
  else {
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(float)),
                                                     "REAL" ) ); // FLOAT(63)
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(double)),
                                                     "DOUBLE PRECISION" ) ); // FLOAT(126)
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long double)),
                                                     "DOUBLE PRECISION" ) ); // FLOAT(126)
  }

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(std::string)),
                                                   "VARCHAR2(4000)" ) ); // To accommodate strings which are really long.

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)),
                                                   "BLOB" ) ); // To accommodate strings which are really long.

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Date)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForType<coral::Date>(),
                                                   "DATE" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForType<coral::TimeStamp>(),
                                                   "TIMESTAMP" ) );

  // Set the current to default
  m_currentCppToSqlMapping = m_defaultCppToSqlMapping;

  // Define here the default SQL -> C++ mappings.
  m_supportedSqlTypes.insert( "NUMBER" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMBER",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );
  m_supportedSqlTypes.insert( "NUMBER(1)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMBER(1)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(bool)) ) );

  m_supportedSqlTypes.insert( "CHAR(1)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "CHAR(1)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char)) ) );

  m_supportedSqlTypes.insert( "VARCHAR2(4000)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "VARCHAR2(4000)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "FLOAT(126)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT(126)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );

  m_supportedSqlTypes.insert( "FLOAT(63)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT(63)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(float)) ) );

  if ( majorVersion > 9 ) {
    m_supportedSqlTypes.insert( "BINARY_DOUBLE" );
    m_defaultSqlToCppMapping.insert( std::make_pair( "BINARY_DOUBLE",
                                                     coral::AttributeSpecification::typeNameForId(typeid(double)) ) );

    m_supportedSqlTypes.insert( "BINARY_FLOAT" );
    m_defaultSqlToCppMapping.insert( std::make_pair( "BINARY_FLOAT",
                                                     coral::AttributeSpecification::typeNameForId(typeid(float)) ) );
  }

  m_supportedSqlTypes.insert( "NUMBER(10)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMBER(10)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "NUMBER(20)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMBER(20)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long long)) ) );

  m_supportedSqlTypes.insert( "NUMBER(5)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMBER(5)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(short)) ) );

  m_supportedSqlTypes.insert( "CLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "CLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  /*
  m_supportedSqlTypes.insert( "LONG" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "LONG",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );
  */

  m_supportedSqlTypes.insert( "BLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) ) );

  m_supportedSqlTypes.insert( "DATE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DATE",
                                                   coral::AttributeSpecification::typeNameForType<coral::Date>() ) );

  m_supportedSqlTypes.insert( "TIMESTAMP" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TIMESTAMP",
                                                   coral::AttributeSpecification::typeNameForType<coral::TimeStamp>() ) );

  // Set the current to default
  m_currentSqlToCppMapping = m_defaultSqlToCppMapping;
}


std::set<std::string>
coral::OracleAccess::TypeConverter::supportedSqlTypes() const
{
  return m_supportedSqlTypes;
}


std::set<std::string>
coral::OracleAccess::TypeConverter::supportedCppTypes() const
{
  return m_supportedCppTypes;
}


std::string
coral::OracleAccess::TypeConverter::defaultCppTypeForSqlType( const std::string& sqlType ) const
{
  std::map<std::string,std::string>::iterator iMapping = m_defaultSqlToCppMapping.find( sqlType );

  if ( iMapping == m_defaultSqlToCppMapping.end() ) {
    if ( ! this->deduceCppType( sqlType ) )
      throw coral::UnSupportedSqlTypeException( m_properties.service()->name(),
                                                "ITypeConverter::defaultCppTypeForSqlType",
                                                sqlType );
    iMapping = m_defaultSqlToCppMapping.find( sqlType );
  }

  return iMapping->second;
}


std::string
coral::OracleAccess::TypeConverter::cppTypeForSqlType( const std::string& sqlType ) const
{
  std::map<std::string,std::string>::iterator iMapping = m_currentSqlToCppMapping.find( sqlType );

  if ( iMapping == m_currentSqlToCppMapping.end() ) {
    if ( ! this->deduceCppType( sqlType ) )
      throw coral::UnSupportedSqlTypeException( m_properties.service()->name(),
                                                "ITypeConverter::cppTypeForSqlType",
                                                sqlType );
    iMapping = m_currentSqlToCppMapping.find( sqlType );
  }

  return iMapping->second;
}


void
coral::OracleAccess::TypeConverter::setCppTypeForSqlType( const std::string& cppType,
                                                          const std::string& sqlType )
{
  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() )
    throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                              "ITypeConverter::setCppTypeForSQLType",
                                              cppType );
  m_currentSqlToCppMapping[ sqlType ] = cppType;
  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() ) {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
  }
}


std::string
coral::OracleAccess::TypeConverter::defaultSqlTypeForCppType( const std::string& cppType ) const
{
  std::map<std::string,std::string>::const_iterator iMapping = m_defaultCppToSqlMapping.find( cppType );
  if ( iMapping == m_defaultCppToSqlMapping.end() )
    throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                              "ITypeConverter::defaultSqlTypeForCppType",
                                              cppType );
  return iMapping->second;
}


std::string
coral::OracleAccess::TypeConverter::sqlTypeForCppType( const std::string& cppType ) const
{
  std::map<std::string,std::string>::const_iterator iMapping = m_currentCppToSqlMapping.find( cppType );
  if ( iMapping == m_currentCppToSqlMapping.end() )
    throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                              "ITypeConverter::sqlTypeForCppType",
                                              cppType );
  return iMapping->second;
}


void
coral::OracleAccess::TypeConverter::setSqlTypeForCppType( const std::string& sqlType,
                                                          const std::string& cppType )
{
  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() )
    throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                              "ITypeConverter::setSqlTypeForCppType",
                                              cppType ); ;
  m_currentCppToSqlMapping[ cppType ] = sqlType;
  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() ) {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
    m_currentSqlToCppMapping[ sqlType ] = cppType;
  }
}


bool
coral::OracleAccess::TypeConverter::deduceCppType( const std::string& sqlType ) const
{
  // No check on the input
  m_supportedSqlTypes.insert( sqlType );

  // Chech for CHAR, VARCHAR and VARCHAR2 types
  if ( sqlType.find( "CHAR" ) == 0 || sqlType.find( "VARCHAR" ) == 0 ) {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
    return true;
  }

  // Check for FLOAT types
  if ( sqlType.find( "FLOAT" ) == 0 ) {
    int precision = 0;
    std::istringstream is( sqlType.substr( 6, sqlType.find( ")" ) - 6 ).c_str() );
    is >> precision;

    if ( precision <= 63 ) {
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(float));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(float));
    }
    else {
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
    }
    return true;
  }

  // Check for NUMBER types
  if ( sqlType.find( "NUMBER" ) == 0 ) {
    std::string tempString = sqlType.substr( 7, sqlType.find( ")" ) - 7 );
    std::string::size_type ip = tempString.find( "," );
    if ( ip == std::string::npos ) {
      int precision = 0;
      std::istringstream is( tempString.c_str() );
      is >> precision;
      std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(short));
      if ( precision > 5 ) {
        if ( precision < 10 ) cppType = coral::AttributeSpecification::typeNameForId(typeid(int));
        else cppType = coral::AttributeSpecification::typeNameForId(typeid(long long));
      }
      m_currentSqlToCppMapping[sqlType] = cppType;
      m_defaultSqlToCppMapping[sqlType] = cppType;
    }
    else {
      int precision = 0;
      int scale = 0;
      std::istringstream isPrecision( tempString.substr(0,ip).c_str() );
      isPrecision >> precision;
      std::istringstream isScale( tempString.substr( ip + 1 ).c_str() );
      isScale >> scale;
      if ( scale > 0 ) {
        std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(double));
        if ( precision < 10 )
          cppType = coral::AttributeSpecification::typeNameForId(typeid(float));
        m_currentSqlToCppMapping[sqlType] = cppType;
        m_defaultSqlToCppMapping[sqlType] = cppType;
      }
      else {
        std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(short));
        if ( precision > 5 ) {
          if ( precision < 10 ) cppType = coral::AttributeSpecification::typeNameForId(typeid(int));
          else cppType = coral::AttributeSpecification::typeNameForId(typeid(long long));
        }
        m_currentSqlToCppMapping[sqlType] = cppType;
        m_defaultSqlToCppMapping[sqlType] = cppType;
      }
    }
    return true;
  }

  // Check for NUMBER types
  if ( sqlType.find( "TIMESTAMP" ) == 0 ) {
    std::string cppType = coral::AttributeSpecification::typeNameForType<coral::TimeStamp>();
    m_currentSqlToCppMapping[sqlType] = cppType;
    m_defaultSqlToCppMapping[sqlType] = cppType;
  }

  std::string cppType = coral::AttributeSpecification::typeNameForId( typeid(std::string) );
  m_currentSqlToCppMapping[sqlType] = cppType;
  m_defaultSqlToCppMapping[sqlType] = cppType;

  coral::MessageStream log( m_properties.service()->name() );
  log << coral::Warning << "Treating SQL type \"" << sqlType
      << "\" as C++ type \"" << cppType << "\"" << coral::MessageStream::endmsg;

  return true;
}
