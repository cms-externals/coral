#include <iostream>
#include "TypeConverter.h"
#include "DomainProperties.h"

#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

#include "RelationalAccess/SessionException.h"

#include "CoralKernel/Service.h"

#include <sstream>
#include <iostream>

static const std::string emptyString = "";

coral::SQLiteAccess::TypeConverter::TypeConverter(const coral::SQLiteAccess::DomainProperties& properties) :
  m_properties( properties ),
  m_supportedCppTypes(),
  m_supportedSqlTypes(),
  m_defaultCppToSqlMapping(),
  m_defaultSqlToCppMapping(),
  m_currentCppToSqlMapping(),
  m_currentSqlToCppMapping()
{

  // Define here the default C++ -> SQL mappings (Whatever the AttributeList supports...).
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(bool)));
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(bool)),
                                                  "BOOLEAN" ) );

  m_supportedCppTypes.insert(coral::AttributeSpecification::typeNameForId(typeid(short))) ;
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(short)),
                                                  "SHORT" ) );

  m_supportedCppTypes.insert(coral::AttributeSpecification::typeNameForId(typeid(unsigned short))) ;
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(unsigned short)) ,
                                                  "UNSIGNEDSHORT" ) );

  m_supportedCppTypes.insert(coral::AttributeSpecification::typeNameForId(typeid(int)));
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(int)),
                                                  "INT" ) );
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned int))) ;
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(unsigned int)) ,"UNSIGNEDINT" ) );

  std::string typeNameForLong = "SLONGLONG";
  std::string typeNameForULong = "ULONGLONG";
  if ( sizeof(long) < sizeof(long long) ) {
    typeNameForLong = "LONG";
    typeNameForULong = "UNSIGNEDLONG";
  }

  m_supportedCppTypes.insert(coral::AttributeSpecification::typeNameForId(typeid(long)));
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(long)) ,typeNameForLong ) );

  m_supportedCppTypes.insert(coral::AttributeSpecification::typeNameForId(typeid(unsigned long)));
  m_defaultCppToSqlMapping.insert( std::make_pair(coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) ,typeNameForULong ) );

  ///64bits int
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned long long)),"ULONGLONG" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long long)));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long long)),"SLONGLONG" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(float)) );
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(float)),
                                                   "FLOAT" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(double)));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(double)),
                                                   "DOUBLE" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(long double)));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(long double)),
                                                   "LONGDOUBLE" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(std::string) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(std::string)),
                                                   "TEXT" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(char) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(char)),
                                                   "CHAR" ) );
  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(unsigned char) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(unsigned char)),
                                                   "UNSIGNEDCHAR" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(coral::Blob)),
                                                   "BLOB" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::Date) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(coral::Date)),
                                                   "DATE" ) );

  m_supportedCppTypes.insert( coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp) ));
  m_defaultCppToSqlMapping.insert( std::make_pair( coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp)),
                                                   "TIMESTAMP" ) );
  // Set the current to default
  m_currentCppToSqlMapping = m_defaultCppToSqlMapping;

  // Define here the default SQL -> C++ mappings.
  m_supportedSqlTypes.insert( "INT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "INT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(int))));

  m_supportedSqlTypes.insert( "UNSIGNEDINT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "UNSIGNEDINT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned int))));

  m_supportedSqlTypes.insert( "SHORT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SHORT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(short) ) ));

  m_supportedSqlTypes.insert( "UNSIGNEDSHORT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "UNSIGNEDSHORT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned short) ) ));

  m_supportedSqlTypes.insert( "LONG" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "LONG",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long) ) ));

  m_supportedSqlTypes.insert( "UNSIGNEDLONG" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "UNSIGNEDLONG",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned long)) ) );

  m_supportedSqlTypes.insert( "ULONGLONG" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "ULONGLONG",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned long long))));

  m_supportedSqlTypes.insert( "SLONGLONG" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "SLONGLONG",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long long) ) ));

  m_supportedSqlTypes.insert( "BOOLEAN" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BOOLEAN",
                                                   coral::AttributeSpecification::typeNameForId(typeid(bool) ) ));

  m_supportedSqlTypes.insert( "TEXT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TEXT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(std::string) )) );

  m_supportedSqlTypes.insert( "FLOAT" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "FLOAT",
                                                   coral::AttributeSpecification::typeNameForId(typeid(float) ) ));

  m_supportedSqlTypes.insert( "DOUBLE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DOUBLE",
                                                   coral::AttributeSpecification::typeNameForId(typeid(double) ) ));

  m_supportedSqlTypes.insert( "LONGDOUBLE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "LONGDOUBLE",
                                                   coral::AttributeSpecification::typeNameForId(typeid(long double)) ) );

  m_supportedSqlTypes.insert( "CHAR" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "CHAR",
                                                   coral::AttributeSpecification::typeNameForId(typeid(char) ) ));

  m_supportedSqlTypes.insert( "UNSIGNEDCHAR" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "UNSIGNEDCHAR",
                                                   coral::AttributeSpecification::typeNameForId(typeid(unsigned char) ) ));

  m_supportedSqlTypes.insert( "BLOB" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "BLOB",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Blob) ) ));

  m_supportedSqlTypes.insert( "DATE" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "DATE",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::Date) ) ));

  m_supportedSqlTypes.insert( "TIMESTAMP" );
  m_defaultSqlToCppMapping.insert( std::make_pair( "TIMESTAMP",
                                                   coral::AttributeSpecification::typeNameForId(typeid(coral::TimeStamp) ) ));
  // Set the current to default
  m_currentSqlToCppMapping = m_defaultSqlToCppMapping;
}

std::set<std::string>
coral::SQLiteAccess::TypeConverter::supportedSqlTypes() const
{

  return m_supportedSqlTypes;
}

std::set<std::string>
coral::SQLiteAccess::TypeConverter::supportedCppTypes() const
{

  return m_supportedCppTypes;
}

std::string
coral::SQLiteAccess::TypeConverter::defaultCppTypeForSqlType( const std::string& sqlType ) const {

  std::map<std::string,std::string>::iterator iMapping = m_defaultSqlToCppMapping.find( sqlType );

  if ( iMapping == m_defaultSqlToCppMapping.end() ) {
    return emptyString;
  }

  return iMapping->second;
}


std::string
coral::SQLiteAccess::TypeConverter::cppTypeForSqlType( const std::string& sqlType ) const
{

  std::map<std::string,std::string>::iterator iMapping = m_currentSqlToCppMapping.find( sqlType );

  if ( iMapping == m_currentSqlToCppMapping.end() ) {
    return emptyString;
  }
  return iMapping->second;
}

void
coral::SQLiteAccess::TypeConverter::setCppTypeForSqlType( const std::string& cppType,
                                                          const std::string& sqlType )
{

  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() ) throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                                                                                                    "ITypeConverter::setCppTypeForSQLType",
                                                                                                                    cppType );
  m_currentSqlToCppMapping[ sqlType ] = cppType;
  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() ) {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
  }
}

std::string
coral::SQLiteAccess::TypeConverter::defaultSqlTypeForCppType( const std::string& cppType ) const
{

  std::map<std::string,std::string>::const_iterator iMapping = m_defaultCppToSqlMapping.find( cppType );
  if ( iMapping == m_defaultCppToSqlMapping.end() ) return emptyString;
  else return iMapping->second;
}

std::string
coral::SQLiteAccess::TypeConverter::sqlTypeForCppType( const std::string& cppType ) const
{

  std::map<std::string,std::string>::const_iterator iMapping = m_currentCppToSqlMapping.find( cppType );
  if ( iMapping == m_currentCppToSqlMapping.end() ) return emptyString;
  else return iMapping->second;
}


void
coral::SQLiteAccess::TypeConverter::setSqlTypeForCppType( const std::string& sqlType,
                                                          const std::string& cppType )
{

  if ( m_supportedCppTypes.find( cppType ) == m_supportedCppTypes.end() ) throw coral::UnSupportedCppTypeException( m_properties.service()->name(),
                                                                                                                    "ITypeConverter::setSqlTypeForCppType",
                                                                                                                    cppType );
  m_currentCppToSqlMapping[ cppType ] = sqlType;
  if ( m_defaultSqlToCppMapping.find( sqlType ) == m_defaultSqlToCppMapping.end() ) {
    m_defaultSqlToCppMapping[ sqlType ] = cppType;
    m_currentSqlToCppMapping[ sqlType ] = cppType;
  }
}
