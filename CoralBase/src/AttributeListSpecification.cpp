#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/AttributeListException.h"
#include "CoralBase/AttributeListSpecification.h"


coral::AttributeListSpecification::AttributeListSpecification() :
  m_counter( 1 ),
  m_attributeSpecifications(),
  m_mapOfNameToIndex()
{
}


coral::AttributeListSpecification::AttributeListSpecification( const coral::AttributeListSpecification& rhs ) :
  m_counter( 1 ),
  m_attributeSpecifications(),
  m_mapOfNameToIndex( rhs.m_mapOfNameToIndex )
{
  for ( std::vector< coral::AttributeSpecification* >::const_iterator iSpecification = rhs.m_attributeSpecifications.begin();
        iSpecification != rhs.m_attributeSpecifications.end(); ++iSpecification ) {
    m_attributeSpecifications.push_back( new coral::AttributeSpecification( **iSpecification ) );
  }
}


coral::AttributeListSpecification::~AttributeListSpecification()
{
  for ( std::vector< coral::AttributeSpecification* >::iterator iSpecification = m_attributeSpecifications.begin();
        iSpecification != m_attributeSpecifications.end(); ++iSpecification ) delete *iSpecification;
}


void
coral::AttributeListSpecification::addRef() const
{
  ++m_counter;
}


void
coral::AttributeListSpecification::release() const
{
  --m_counter;
  if ( m_counter == 0 ) delete this;
}


void
coral::AttributeListSpecification::extend( const std::string& name,
                                           const std::type_info& type )
{
  if ( m_mapOfNameToIndex.find( name ) != m_mapOfNameToIndex.end() )
    throw coral::AttributeListException( "An attribute with the name \"" + name + "\" exists already" );
  m_attributeSpecifications.push_back( new coral::AttributeSpecification( name, type ) );
  m_mapOfNameToIndex.insert( std::make_pair( name, static_cast<int>( m_attributeSpecifications.size() - 1 ) ) );
}


void
coral::AttributeListSpecification::extend( const std::string& name,
                                           const std::string& typeName )
{
  const std::type_info* info = coral::AttributeSpecification::typeIdForName( typeName );
  if(info)
    this->extend( name, *( info ) );
  else
    // This will not fix bug #54968 because this should never happen
    // (AttributeSpecification::typeIdForName throws if it finds no match)!
    throw coral::AttributeListException( "PANIC! The typeID for \"" + typeName + "\" is not valid" );
}


int
coral::AttributeListSpecification::index( const std::string& name ) const
{
  std::map<std::string, int>::const_iterator iSpecificationForAttribute = m_mapOfNameToIndex.find( name );
  if ( iSpecificationForAttribute == m_mapOfNameToIndex.end() ) return -1;
  else return iSpecificationForAttribute->second;
}


const coral::AttributeSpecification&
coral::AttributeListSpecification::specificationForAttribute( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( m_attributeSpecifications.size() ) )
    throw coral::AttributeListException( "Invalid index for the attribute already" );
  return *( m_attributeSpecifications[ index ] );
}


bool
coral::AttributeListSpecification::operator==( const coral::AttributeListSpecification& rhs ) const
{
  if ( this->size() != rhs.size() ) return false;
  size_t theSize = this->size();
  for ( size_t i = 0; i < theSize; ++i )
    if ( (*this)[i] != rhs[i] )
      return false;
  return true;
}
