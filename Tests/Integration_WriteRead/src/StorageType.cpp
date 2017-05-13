// $Id: StorageType.cpp,v 1.3 2011/03/22 12:55:44 avalassi Exp $

// Include files
#include <sstream>
#include <stdexcept>
#include "StorageType.h"
#include "types.h"

//-----------------------------------------------------------------------------

const std::string cool::StorageType::name() const
{
  switch ( m_id ) {
  case Bool:      return "Bool";
    //case Char:      return "Char";
  case UChar:     return "UChar";
  case Int16:     return "Int16";
  case UInt16:    return "UInt16";
  case Int32:     return "Int32";
  case UInt32:    return "UInt32";
  case UInt63:    return "UInt63";
  case Int64:     return "Int64";
    //case UInt64:    return "UInt64";
  case Float:     return "Float";
  case Double:    return "Double";
  case String255: return "String255";
  case String4k:  return "String4k";
  case String64k: return "String64k";
  case String16M: return "String16M";
  case Blob64k:   return "Blob64k";
  case Blob16M:   return "Blob16M";
  }
  std::stringstream out;
  out << "PANIC! Unknown type '" << m_id << "' in StorageType::name()";
  throw std::runtime_error( out.str() );
}

//-----------------------------------------------------------------------------

const std::type_info& cool::StorageType::cppType() const
{
  switch ( m_id ) {
  case Bool:      return typeid( cool::Bool );
    //case Char:      return typeid( Char );
  case UChar:     return typeid( cool::UChar );
  case Int16:     return typeid( cool::Int16 );
  case UInt16:    return typeid( cool::UInt16 );
  case Int32:     return typeid( cool::Int32 );
  case UInt32:    return typeid( cool::UInt32 );
  case UInt63:    return typeid( cool::UInt63 );
  case Int64:     return typeid( cool::Int64 );
    //case UInt64:    return typeid( cool::UInt64 );
  case Float:     return typeid( cool::Float );
  case Double:    return typeid( cool::Double );
  case String255: return typeid( cool::String255 );
  case String4k:  return typeid( cool::String4k );
  case String64k: return typeid( cool::String64k );
  case String16M: return typeid( cool::String16M );
  case Blob64k:   return typeid( cool::Blob64k );
  case Blob16M:   return typeid( cool::Blob16M );
  }
  std::stringstream out;
  out << "PANIC! Unknown type '" << m_id << "' in StorageType::cppType()";
  throw std::runtime_error( out.str() );
}

//-----------------------------------------------------------------------------

size_t cool::StorageType::maxSize() const
{
  switch ( m_id ) {
  case Bool:      return 0;
    //case Char:      return 0;
  case UChar:     return 0;
  case Int16:     return 0;
  case UInt16:    return 0;
  case Int32:     return 0;
  case UInt32:    return 0;
  case UInt63:    return 0;
  case Int64:     return 0;
    //case UInt64:    return 0;
  case Float:     return 0;
  case Double:    return 0;
  case String255: return 255;
  case String4k:  return 4000;
  case String64k: return 65535;
  case String16M: return 16777215;
  case Blob64k:   return 65535;
  case Blob16M:   return 16777215;
  }
  std::stringstream out;
  out << "PANIC! Unknown type '" << m_id << "' in StorageType::maxSize()";
  throw std::runtime_error( out.str() );
}

//-----------------------------------------------------------------------------

const cool::StorageType& cool::StorageType::storageType( const TypeId& id )
{
  switch ( id ) {
  case Bool:
    { static const StorageType type( id ); return type; }
    //case Char:
    //  { static const StorageType type( id ); return type; }
  case UChar:
    { static const StorageType type( id ); return type; }
  case Int16:
    { static const StorageType type( id ); return type; }
  case UInt16:
    { static const StorageType type( id ); return type; }
  case Int32:
    { static const StorageType type( id ); return type; }
  case UInt32:
    { static const StorageType type( id ); return type; }
  case UInt63:
    { static const StorageType type( id ); return type; }
  case Int64:
    { static const StorageType type( id ); return type; }
    //case UInt64:
    //  { static const StorageType type( id ); return type; }
  case Float:
    { static const StorageType type( id ); return type; }
  case Double:
    { static const StorageType type( id ); return type; }
  case String255:
    { static const StorageType type( id ); return type; }
  case String4k:
    { static const StorageType type( id ); return type; }
  case String64k:
    { static const StorageType type( id ); return type; }
  case String16M:
    { static const StorageType type( id ); return type; }
  case Blob64k:
    { static const StorageType type( id ); return type; }
  case Blob16M:
    { static const StorageType type( id ); return type; }
  }
  std::stringstream out;
  out << "PANIC! Unknown type '" << id << "' in StorageType::storageType()";
  throw std::runtime_error( out.str() );
}

//-----------------------------------------------------------------------------

/*
void cool::StorageType::validate( const std::type_info& cppTypeOfData,
                                  const void* addressOfData,
                                  const std::string& variableName ) const
{
  if ( cppType() != cppTypeOfData )
    throw StorageTypeWrongCppType
      ( variableName, *this, cppTypeOfData, "StorageType" );
  switch( id() ) {
  case StorageType::String255:
  case StorageType::String4k:
  case StorageType::String64k:
  case StorageType::String16M:
    {
      const std::string& data =
        *( static_cast<const std::string*>( addressOfData ) );
      // Check that string values are not longer than the allowed maximum size
      size_t size = data.size(); // std::string::size() returns size_t
      if ( maxSize() < size )
        throw StorageTypeStringTooLong
          ( variableName, *this, size, "StorageType" );
      // Check that strings do not contain the character '\0' - fix bug #22385
      if ( data.find('\0') != std::string::npos )
        throw StorageTypeStringContainsNullChar
          ( variableName, *this, "StorageType" );
    }
    break;
  case StorageType::Blob64k:
  case StorageType::Blob16M:
    {
      const coral::Blob& data =
        *( static_cast<const coral::Blob*>( addressOfData ) );
      // Check that blob values are not longer than the allowed maximum size
      long size = data.size(); // coral::Blob::size() returns long
      if ( (long)maxSize() < size )
        throw StorageTypeBlobTooLong
          ( variableName, *this, size, "StorageType" );
    }
    break;
  case StorageType::UInt63:
    {
      cool::UInt63 data =
        *( static_cast<const cool::UInt63*>( addressOfData ) );
      // Check that UInt63 values are in the range [0, 2^63-1]
      //if ( ( data & 0x8000000000000000ULL ) != 0ULL ) // NO - use min/max
      //if ( data < UInt63Min || data > UInt63Max )     // NO - data never < 0
      if ( data > UInt63Max ) // Only check max...
        throw StorageTypeInvalidUInt63
          ( variableName, data, "StorageType" );
    }
    break;
  default:
    // All other types are always ok
    break;
  }
}
*/

 //-----------------------------------------------------------------------------
