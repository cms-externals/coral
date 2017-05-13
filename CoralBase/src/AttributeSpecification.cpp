#include "CoralBase/AttributeException.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
coral::AttributeSpecification::AttributeSpecification( const std::string& name, const std::type_info& type ) :
  m_name( name ),
  m_type( AttributeSpecification::validateType( type ) )
{
}


coral::AttributeSpecification::~AttributeSpecification()
{
}


std::string coral::AttributeSpecification::typeNameForId( const std::type_info& type )
{
  if ( type == typeid(bool) ) return "bool";
  else if ( type == typeid(char) ) return "char";
  else if ( type == typeid(unsigned char) ) return "unsigned char";
  else if ( type == typeid(signed char) ) return "char";
  else if ( type == typeid(short) ) return "short";
  else if ( type == typeid(unsigned short) ) return "unsigned short";
  else if ( type == typeid(int) ) return "int";
  else if ( type == typeid(unsigned int) ) return "unsigned int";
  else if ( type == typeid(long) ) return "long";
  else if ( type == typeid(unsigned long) ) return "unsigned long";
  else if ( type == typeid(long long) ) return "long long";
  else if ( type == typeid(unsigned long long) ) return "unsigned long long";
  else if ( type == typeid(float) ) return "float";
  else if ( type == typeid(double) ) return "double";
  else if ( type == typeid(long double) ) return "long double";
  else if ( type == typeid(std::string) ) return "string";
  else if ( type == typeid(coral::Blob) ) return "blob";
  else if ( type == typeid(coral::Date) ) return "date";
  else if ( type == typeid(coral::TimeStamp) ) return "time stamp";

  // Type not supported
  throw coral::AttributeException( "Type \"" + std::string( type.name() ) + "\" is not supported" );
  return "";
}


const std::type_info*
coral::AttributeSpecification::typeIdForName( const std::string& typeName )
{
  if ( typeName == "bool" ) return &( typeid(bool) );
  else if ( typeName == "char" ) return &( typeid(char) );
  else if ( typeName == "unsigned char" ) return &( typeid(unsigned char) );
  else if ( typeName == "signed char" ) return &( typeid(char) );
  else if ( typeName == "short" ) return &( typeid(short) );
  else if ( typeName == "unsigned short" ) return &( typeid(unsigned short) );
  else if ( typeName == "int" ) return &( typeid(int) );
  else if ( typeName == "unsigned int" ) return &( typeid(unsigned int) );
  else if ( typeName == "long" ) return &( typeid(long) );
  else if ( typeName == "unsigned long" ) return &( typeid(unsigned long) );
  else if ( typeName == "long long" ) return &( typeid(long long) );
  else if ( typeName == "unsigned long long" ) return &( typeid(unsigned long long) );
  else if ( typeName == "float" ) return &( typeid(float) );
  else if ( typeName == "double" ) return &( typeid(double) );
  else if ( typeName == "long double" ) return &( typeid(long double) );
  else if ( typeName == "string" ) return &( typeid(std::string) );
  else if ( typeName == "blob" ) return &( typeid(coral::Blob) );
  else if ( typeName == "date" ) return &( typeid(coral::Date) );
  else if ( typeName == "time stamp" ) return &( typeid(coral::TimeStamp) );

  // Type not supported
  throw coral::AttributeException( "Type \"" + typeName + "\" is not supported" );

  return 0;
}


const std::type_info*
coral::AttributeSpecification::validateType( const std::type_info& type )
{
  if ( type == typeid(bool) ||
       type == typeid(char) ||
       type == typeid(unsigned char) ||
       type == typeid(short) ||
       type == typeid(unsigned short) ||
       type == typeid(int) ||
       type == typeid(unsigned int) ||
       type == typeid(long) ||
       type == typeid(unsigned long) ||
       type == typeid(long long) ||
       type == typeid(unsigned long long) ||
       type == typeid(float) ||
       type == typeid(double) ||
       type == typeid(long double) ||
       type == typeid(std::string) ||
       type == typeid(coral::Blob) ||
       type == typeid(coral::Date) ||
       type == typeid(coral::TimeStamp) ) return &type;
  //
  //here we assume char type is always signed following original coral assumption.
  //
  if (type == typeid(signed char)) return &(typeid(char));
  // Type not supported
  throw coral::AttributeException( "Type \"" + std::string( type.name() ) + "\" is not supported" );
  return 0;
}
