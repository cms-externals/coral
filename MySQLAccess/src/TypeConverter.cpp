// $Id: TypeConverter.cpp,v 1.22 2011/03/22 10:29:55 avalassi Exp $
#include "MySQL_headers.h"

#include <iostream>
#include <sstream>
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"
#include "RelationalAccess/SessionException.h"

#include "TypeConverter.h"

static const std::string emptyString = "";

coral::MySQLAccess::TypeConverter::TypeConverter()
  :   m_supportedCppTypes(),
      m_supportedSqlTypes(),
      m_defaultCppToSqlMapping(),
      m_defaultSqlToCppMapping(),
      m_currentCppToSqlMapping(),
      m_currentSqlToCppMapping()
{
  initialize();
}

std::set<std::string> coral::MySQLAccess::TypeConverter::supportedSqlTypes() const
{
  return m_supportedSqlTypes;
}

std::set<std::string> coral::MySQLAccess::TypeConverter::supportedCppTypes() const
{
  return m_supportedCppTypes;
}

std::string coral::MySQLAccess::TypeConverter::defaultCppTypeForSqlType( const std::string& sqlType ) const
{
  std::map<std::string,std::string>::iterator iMapping = m_defaultSqlToCppMapping.find( sqlType );

  if ( iMapping == m_defaultSqlToCppMapping.end() ) {
    if ( ! this->deduceCppType( sqlType ) )
      return emptyString;
    else
      iMapping = m_defaultSqlToCppMapping.find( sqlType );
  }

  return iMapping->second;
}


std::string coral::MySQLAccess::TypeConverter::cppTypeForSqlType( const std::string& sqlType ) const
{
  std::map<std::string,std::string>::iterator iMapping = m_currentSqlToCppMapping.find( sqlType );

  if ( iMapping == m_currentSqlToCppMapping.end() ) {
    if ( ! this->deduceCppType( sqlType ) )
      return emptyString;
    else
      iMapping = m_currentSqlToCppMapping.find( sqlType );
  }

  return iMapping->second;
}

void coral::MySQLAccess::TypeConverter::setCppTypeForSqlType( const std::string& cppType, const std::string& sqlType )
{
  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() )
    throw coral::UnSupportedCppTypeException( "coral::MySQLAccess::TypeConverter", "ITypeConverter::setCppTypeForSQLType", cppType );

  m_currentSqlToCppMapping[ sqlType ] = cppType;

  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() )
  {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
  }
}

std::string coral::MySQLAccess::TypeConverter::defaultSqlTypeForCppType( const std::string& cppType ) const
{
  std::map<std::string,std::string>::const_iterator iMapping = m_defaultCppToSqlMapping.find( cppType );

  if ( iMapping == m_defaultCppToSqlMapping.end() )
    return emptyString;
  else
    return iMapping->second;
}

std::string coral::MySQLAccess::TypeConverter::sqlTypeForCppType( const std::string& cppType ) const
{
  std::map<std::string,std::string>::const_iterator iMapping = m_currentCppToSqlMapping.find( cppType );

  if ( iMapping == m_currentCppToSqlMapping.end() )
    return emptyString;
  else
    return iMapping->second;
}


void coral::MySQLAccess::TypeConverter::setSqlTypeForCppType( const std::string& sqlType, const std::string& cppType )
{
  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() )
    throw coral::UnSupportedCppTypeException( "coral::MySQLAccess::TypeConverter", "ITypeConverter::setSqlTypeForCppType", cppType );

  m_currentCppToSqlMapping[ cppType ] = sqlType;

  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() )
  {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
    m_currentSqlToCppMapping[ sqlType ] = cppType;
  }
}

bool coral::MySQLAccess::TypeConverter::deduceCppType( const std::string& sqlType ) const
{
  // No check on the input
  m_supportedSqlTypes.insert( sqlType );

  // Check for DATE, TIMESTAMP
  if ( sqlType.find( "DATE" ) == 0 )
  {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::Date));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::Date));
    return true;
  }

  if ( sqlType.find( "TIMESTAMP" ) == 0 )
  {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp));
    return true;
  }

  if ( sqlType.find( "BLOB" ) != std::string::npos )
  {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::Blob));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(coral::Blob));
    return true;
  }

  // Check for CHAR, VARCHAR and VARCHAR2 types
  if ( sqlType.find( "VARCHAR" ) == 0 || sqlType.find( "TEXT") == 0 )
  {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
    return true;
  }

  if ( sqlType.find( "CHAR" ) == 0 )
  {
    int precision = 0;

    std::string sprecision;

    if( sqlType.size() < 5 )
      sprecision = "0";
    else
      sprecision = sqlType.substr( 5, sqlType.find( ")" ) - 5 );

    std::istringstream is( sprecision );
    is >> precision;

    if( precision == 1 || precision == 0 )
    {
      m_supportedSqlTypes.insert( sqlType );
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
    }
    else
    {
      m_supportedSqlTypes.insert( sqlType );
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(std::string));
    }

    return true;
  }

  // Check for DOUBLE
  if ( sqlType.find( "DOUBLE" ) == 0 )
  {
    m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
    m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
    return true;
  }

  // Check for FLOAT types
  if ( sqlType.find( "FLOAT" ) == 0 )
  {
    int precision = 0;

    std::string sprecision;

    if( sqlType.size() < 6 )
      sprecision = "0";
    else
      sprecision = sqlType.substr( 6, sqlType.find( ")" ) - 6 );

    std::istringstream is( sprecision );
    is >> precision;

    if ( precision < 53 )
    {
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(float));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(float));
    }
    else
    {
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(double));
    }

    return true;
  }

  // Handle unsigned char and bool which clash due to TINYINT SQL type
  if ( sqlType.find( "TINYINT" ) == 0 )
  {
    int precision = 0;
    std::string sprecision;

    if( sqlType.size() < 8 )
      sprecision = "0";
    else
      sprecision = sqlType.substr( 8, sqlType.find( ")" ) - 8 );

    std::istringstream is( sprecision );
    is >> precision;

    if ( precision == 1 )
    {
      // BOOL-ean
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(bool));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(bool));
    }
    else if ( precision == 3 ) // aka TINTYINT(3) UNSIGNED
    {
      // unsigned char
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(unsigned char));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(unsigned char));
    }
    else if ( precision == 4 ) // That's what describe shows, aka TINYINT(4), 1st bit is used for sign
    {
      // char
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
    }
    else
    {
      // Assume precision 0, pure TINYINT, never happens in reality
      m_currentSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
      m_defaultSqlToCppMapping[sqlType] = coral::AttributeSpecification::typeNameForId(typeid(char));
    }

    return true;
  }

  // Check for integer types
  if ( sqlType.find( "INT" ) == 0 || sqlType.find( "INTEGER" ) == 0 )
  {
    std::string cppType;

    if ( sqlType.find( "UNSIGNED" ) == 0 )
      cppType = coral::AttributeSpecification::typeNameForId(typeid(unsigned int));
    else
      cppType = coral::AttributeSpecification::typeNameForId(typeid(int));

    m_currentSqlToCppMapping[sqlType] = cppType;
    m_defaultSqlToCppMapping[sqlType] = cppType;

    return true;
  }

  // Check for NUMBER types
  if ( sqlType.find( "NUMERIC" ) == 0 || sqlType.find( "DECIMAL" ) == 0 )
  {
    std::string tempString = sqlType.substr( 7, sqlType.find( ")" ) - 7 );
    std::string::size_type ip = tempString.find( "," );

    if ( ip == std::string::npos )
    {
      int precision = 0;
      std::istringstream is( tempString.c_str() );
      is >> precision;
      std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(short));

      if ( precision > 5 ) {
        if ( precision < 10 )
          cppType = coral::AttributeSpecification::typeNameForId(typeid(int));
        else
          cppType = coral::AttributeSpecification::typeNameForId(typeid(long));
      }
      m_currentSqlToCppMapping[sqlType] = cppType;
      m_defaultSqlToCppMapping[sqlType] = cppType;
    }
    else
    {
      int precision = 0;
      int scale = 0;
      std::istringstream isPrecision( tempString.substr(0,ip).c_str() );
      isPrecision >> precision;
      std::istringstream isScale( tempString.substr( ip + 1 ).c_str() );
      isScale >> scale;

      if ( scale > 0 )
      {
        std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(double));

        if ( precision < 10 )
          cppType = coral::AttributeSpecification::typeNameForId(typeid(float));

        m_currentSqlToCppMapping[sqlType] = cppType;
        m_defaultSqlToCppMapping[sqlType] = cppType;
      }
      else
      {
        std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(short));

        if ( precision > 5 ) {
          if ( precision < 10 )
            cppType = coral::AttributeSpecification::typeNameForId(typeid(int));
          else
            cppType = coral::AttributeSpecification::typeNameForId(typeid(long));
        }
        m_currentSqlToCppMapping[sqlType] = cppType;
        m_defaultSqlToCppMapping[sqlType] = cppType;
      }
    }

    return true;
  }

  std::string cppType = coral::AttributeSpecification::typeNameForId(typeid(std::string));
  m_currentSqlToCppMapping[sqlType] = cppType;
  m_defaultSqlToCppMapping[sqlType] = cppType;

  coral::MessageStream log( "coral::MySQLAccess::TypeConverter" );
  log << coral::Warning << "Treating SQL type \"" << sqlType
      << "\" as C++ type \"" << cppType << "\"" << std::endl << coral::MessageStream::endmsg;

  return false;
}

void
coral::MySQLAccess::TypeConverter::initialize()
{
  // FIXME! Check the MySQL server version
  coral::MessageStream log( "MySQLAccess::TypeConverter" );

  log << coral::Debug << "Initializing MySQL type converter" << std::endl << coral::MessageStream::endmsg;

  // Define here the default C++ -> SQL mappings (Whatever the AttributeList supports...).
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(bool)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(bool)),
                                                   "TINYINT(1)" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(char)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(char)),
                                                   "TINYINT" ) );

  //  This mapping does not work well with MySQL 4.0.x as various control caracters cause problems either when written or read
  //    m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(char)) );
  //    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(char)),
  //                                                     "CHAR(1)" ) );

  // We need the latest version of Attribute list to do it
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned char)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned char)),
                                                   "TINYINT UNSIGNED" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(short)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(short)),
                                                   "SMALLINT" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned short)),
                                                   "SMALLINT UNSIGNED" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(int)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(int)),
                                                   "INTEGER" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned int)),
                                                   "INTEGER UNSIGNED" ) );

  if( sizeof(long) < sizeof(long long) )
  {
    m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long)) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long)),
                                                     "INTEGER" ) );

    m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)),
                                                     "INTEGER UNSIGNED" ) );
  }
  else
  {
    m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long)) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long)),
                                                     "BIGINT" ) );

    m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) );
    m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long)),
                                                     "BIGINT UNSIGNED" ) );
  }

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long long)),
                                                   "BIGINT" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)),
                                                   "BIGINT UNSIGNED" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(float)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(float)),
                                                   "FLOAT" ) ); // FLOAT(24)

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(double)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(double)),
                                                   "DOUBLE PRECISION" ) ); // FLOAT(53)

  // The only way to get long strings case-sensitive in 4.0.x
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(std::string)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(std::string)),
                                                   "VARCHAR(255) BINARY" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)),
                                                   "LONGBLOB" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Date)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForType<coral::Date>(),
                                                   "DATE" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForType<coral::TimeStamp>(),
                                                   "TIMESTAMP" ) );

  // Set the current to default
  m_currentCppToSqlMapping = m_defaultCppToSqlMapping;

  // Define here the default SQL -> C++ mappings.
  m_supportedSqlTypes.insert( "NUMERIC" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMERIC",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );
  m_supportedSqlTypes.insert( "TINYINT(1)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TINYINT(1)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(bool)) ) );

  m_supportedSqlTypes.insert( "CHAR" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "CHAR",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char)) ) );

  m_supportedSqlTypes.insert( "CHAR(1)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "CHAR(1)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char)) ) );

  m_supportedSqlTypes.insert( "TINYINT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TINYINT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char)) ) );

  m_supportedSqlTypes.insert( "TINYINT(3) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TINYINT(3) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned char)) ) );

  m_supportedSqlTypes.insert( "TINYINT(4)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TINYINT(4)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char)) ) );

  //   m_supportedSqlTypes.insert( "TINYINT UNSIGNED" );
  //   m_defaultSqlToCppMapping.insert( std::make_pair( "TINYINT UNSIGNED",
  //                                                    coral::AttributeSpecification::typeNameForId(typeid(unsigned char)) ) );

  m_supportedSqlTypes.insert( "VARCHAR(255)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "VARCHAR(255)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "TEXT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TEXT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "MEDIUMTEXT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "MEDIUMTEXT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "LONG VARCHAR" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "LONG VARCHAR",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "VARCHAR(255) BINARY" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "VARCHAR(255) BINARY",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string)) ) );

  m_supportedSqlTypes.insert( "BLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) ) );

  m_supportedSqlTypes.insert( "LONGBLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "LONGBLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) ) );

  m_supportedSqlTypes.insert( "MEDIUMBLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "MEDIUMBLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) ) );

  m_supportedSqlTypes.insert( "TINYBLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TINYBLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)) ) );

  m_supportedSqlTypes.insert( "FLOAT(53)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT(53)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );

  m_supportedSqlTypes.insert( "DOUBLE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DOUBLE",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );

  m_supportedSqlTypes.insert( "DOUBLE PRECISION" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DOUBLE PRECISION",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double)) ) );

  m_supportedSqlTypes.insert( "FLOAT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(float)) ) );

  m_supportedSqlTypes.insert( "FLOAT(24)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT(24)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(float)) ) );

  m_supportedSqlTypes.insert( "DECIMAL(11)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DECIMAL(11)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long)) ) );

  m_supportedSqlTypes.insert( "NUMERIC(11)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "NUMERIC(11)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long)) ) );

  m_supportedSqlTypes.insert( "BIGINT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BIGINT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long long)) ) );

  m_supportedSqlTypes.insert( "BIGINT UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BIGINT UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)) ) );

  m_supportedSqlTypes.insert( "BIGINT(20)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BIGINT(20)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long long)) ) );

  m_supportedSqlTypes.insert( "BIGINT(20) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BIGINT(20) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)) ) );

  m_supportedSqlTypes.insert( "INTEGER" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INTEGER",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "INTEGER(10)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INTEGER(10)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "INT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "INT(11)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT(11)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "INT(10)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT(10)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "MEDIUMINT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "MEDIUMINT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int)) ) );

  m_supportedSqlTypes.insert( "DECIMAL" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DECIMAL",
                                                   coral::AttributeSpecification::typeNameForId(typeid(float)) ) );

  m_supportedSqlTypes.insert( "DECIMAL(22,0)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DECIMAL(22,0)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long long)) ) );

  m_supportedSqlTypes.insert( "INTEGER UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INTEGER UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "INT UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "INT(10) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT(10) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "INT(11) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT(11) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "MEDIUMINT UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "MEDIUMINT UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "DECIMAL UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DECIMAL UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ) );

  m_supportedSqlTypes.insert( "DECIMAL(22,0) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DECIMAL(22,0) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)) ) );

  m_supportedSqlTypes.insert( "SMALLINT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SMALLINT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(short)) ) );

  m_supportedSqlTypes.insert( "SMALLINT(6)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SMALLINT(6)",
                                                   coral::AttributeSpecification::typeNameForId(typeid(short)) ) );

  m_supportedSqlTypes.insert( "SMALLINT UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SMALLINT UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) ) );

  m_supportedSqlTypes.insert( "SMALLINT(5) UNSIGNED" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SMALLINT(5) UNSIGNED",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) ) );

  m_supportedSqlTypes.insert( "DATE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DATE",
                                                   coral::AttributeSpecification::typeNameForType<coral::Date>() ) );

  m_supportedSqlTypes.insert( "TIMESTAMP" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TIMESTAMP",
                                                   coral::AttributeSpecification::typeNameForType<coral::TimeStamp>() ) );

  m_supportedSqlTypes.insert( "TIMESTAMP(14)" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TIMESTAMP(14)",
                                                   coral::AttributeSpecification::typeNameForType<coral::TimeStamp>() ) );

  // Set the current to default
  m_currentSqlToCppMapping = m_defaultSqlToCppMapping;
}
