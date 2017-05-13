#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/AttributeListException.h"
#include <ostream>

coral::AttributeList::AttributeList() :
  m_specification( new coral::AttributeListSpecification ),
  m_ownSpecification( true ),
  m_data()
{
}


coral::AttributeList::AttributeList( const coral::AttributeListSpecification& spec,
                                     bool sharedSpecification ) :
  m_specification( sharedSpecification ? const_cast< coral::AttributeListSpecification* >( &spec ) : new coral::AttributeListSpecification( spec ) ),
  m_ownSpecification( ! sharedSpecification ),
  m_data()
{
  if ( sharedSpecification ) spec.addRef();

  for ( coral::AttributeListSpecification::const_iterator iSpec = m_specification->begin();
        iSpec != m_specification->end(); ++iSpec )
    m_data.push_back( new coral::Attribute( *iSpec ) );
}


coral::AttributeList::AttributeList( const coral::AttributeList& rhs ) :
  m_specification( rhs.m_ownSpecification ? new coral::AttributeListSpecification( *( rhs.m_specification ) ) : rhs.m_specification ),
  m_ownSpecification( rhs.m_ownSpecification ),
  m_data()
{
  if ( ! m_ownSpecification ) m_specification->addRef();

  // Copy the data.
  coral::AttributeListSpecification::const_iterator iSpecification = m_specification->begin();
  for ( std::vector< coral::Attribute* >::const_iterator iAttribute = rhs.m_data.begin();
        iAttribute != rhs.m_data.end(); ++iAttribute, ++iSpecification ) {
    coral::Attribute* attribute = new coral::Attribute( *iSpecification );
    attribute->fastCopy( **iAttribute );
    m_data.push_back( attribute );
  }
}


coral::AttributeList&
coral::AttributeList::operator=( const coral::AttributeList& rhs )
{
  // Clears all data
  for ( std::vector< coral::Attribute* >::iterator iAttribute = m_data.begin();
        iAttribute != m_data.end(); ++iAttribute ) delete *iAttribute;
  m_data.clear();
  m_specification->release();

  // The specification...
  m_specification = rhs.m_ownSpecification ? new coral::AttributeListSpecification( *( rhs.m_specification ) ) : rhs.m_specification;
  m_ownSpecification = rhs.m_ownSpecification;
  if ( ! m_ownSpecification ) m_specification->addRef();

  // Copy the data.
  coral::AttributeListSpecification::const_iterator iSpecification = m_specification->begin();
  for ( std::vector< coral::Attribute* >::const_iterator iAttribute = rhs.m_data.begin();
        iAttribute != rhs.m_data.end(); ++iAttribute, ++iSpecification ) {
    coral::Attribute* attribute = new coral::Attribute( *iSpecification );
    attribute->fastCopy( **iAttribute );
    m_data.push_back( attribute );
  }

  return *this;
}


coral::AttributeList::~AttributeList()
{
  for ( std::vector< coral::Attribute* >::iterator iAttribute = m_data.begin();
        iAttribute != m_data.end(); ++iAttribute ) delete *iAttribute;
  m_specification->release();
}


bool
coral::AttributeList::operator==( const AttributeList& rhs ) const
{
  if ( this->size() != rhs.size() ) return false;
  for ( size_t i = 0; i < this->size(); ++i )
    if ( (*this)[i] != rhs[i] )
      return false;
  return true;
}


void
coral::AttributeList::extend( const std::string& name,
                              const std::string& typeName )
{
  if ( ! m_ownSpecification ) {
    coral::AttributeListSpecification* newSpec = new coral::AttributeListSpecification( *m_specification );
    m_specification->release();
    m_specification = newSpec;
    m_ownSpecification = true;
    // Rebuild the attributes
    int i = 0;
    for ( std::vector< coral::Attribute* >::iterator iAttribute = m_data.begin();
          iAttribute != m_data.end(); ++iAttribute, ++i ) {
      coral::Attribute* newAttribute = new coral::Attribute( m_specification->specificationForAttribute( i ) );
      newAttribute->fastCopy( **iAttribute );
      delete *iAttribute;
      *iAttribute = newAttribute;
    }
  }

  m_specification->extend( name, typeName );
  m_data.push_back( new coral::Attribute( m_specification->specificationForAttribute( m_data.size() ) ) );
}


void
coral::AttributeList::extend( const std::string& name,
                              const std::type_info& type )
{
  if ( ! m_ownSpecification ) {
    coral::AttributeListSpecification* newSpec = new coral::AttributeListSpecification( *m_specification );
    m_specification->release();
    m_specification = newSpec;
    m_ownSpecification = true;
    // Rebuild the attributes
    int i = 0;
    for ( std::vector< coral::Attribute* >::iterator iAttribute = m_data.begin();
          iAttribute != m_data.end(); ++iAttribute, ++i ) {
      coral::Attribute* newAttribute = new coral::Attribute( m_specification->specificationForAttribute( i ) );
      newAttribute->fastCopy( **iAttribute );
      delete *iAttribute;
      *iAttribute = newAttribute;
    }
  }

  m_specification->extend( name, type );
  m_data.push_back( new coral::Attribute( m_specification->specificationForAttribute( m_data.size() ) ) );
}


coral::AttributeList&
coral::AttributeList::merge( const coral::AttributeList& rhs )
{
  for ( coral::AttributeList::const_iterator iAttribute = rhs.begin();
        iAttribute != rhs.end(); ++iAttribute ) {
    const std::string& attributeName = iAttribute->specification().name();

    bool found = false;
    for ( std::vector< coral::Attribute* >::iterator iThisAttribute = m_data.begin();
          iThisAttribute != m_data.end(); ++iThisAttribute ) {
      if ( (*iThisAttribute)->specification().name() == attributeName ) {
        (*iThisAttribute)->shareData( *iAttribute );
        found = true;
        break;
      }
    }
    if ( ! found ) {
      this->extend( attributeName, iAttribute->specification().type() );
      m_data.back()->shareData( *iAttribute );
    }
  }

  return *this;
}


coral::Attribute&
coral::AttributeList::operator[]( const std::string name )
{
  int index = m_specification->index( name );
  if ( index < 0 || index >= static_cast<int>( m_data.size() ) )
    throw coral::AttributeListException( "Variable \"" + name + "\" does not exist in the AttributeList" );
  return *( m_data[index] );
}


const coral::Attribute&
coral::AttributeList::operator[]( const std::string name ) const
{
  int index = m_specification->index( name );
  if ( index < 0 || index >= static_cast<int>( m_data.size() ) )
    throw coral::AttributeListException( "Variable \"" + name + "\" does not exist in the AttributeList" );
  return *( m_data[index] );
}


coral::Attribute&
coral::AttributeList::operator[]( unsigned int index )
{
  if ( index >= m_data.size() )
    throw coral::AttributeListException( "Invalid index specified for accessing a variable in the AttributeList" );
  return *( m_data[index] );
}


const coral::Attribute&
coral::AttributeList::operator[]( unsigned int index ) const
{
  if ( index >= m_data.size() )
    throw coral::AttributeListException( "Invalid index specified for accessing a variable in the AttributeList" );
  return *( m_data[index] );
}


void
coral::AttributeList::copyData( const coral::AttributeList& rhs )
{
  std::vector< coral::Attribute* >::const_iterator iAttributeRhs = rhs.m_data.begin();
  for ( std::vector< coral::Attribute* >::iterator iAttribute = m_data.begin();
        iAttribute != m_data.end() && iAttributeRhs != rhs.m_data.end(); // Copies only the first elements in case of lists with different sizes
        ++iAttribute, ++iAttributeRhs )
  {
    **iAttribute = **iAttributeRhs;
  }
}



void
coral::AttributeList::fastCopyData( const coral::AttributeList& rhs )
{
  std::vector< coral::Attribute* >::iterator idest = m_data.begin();
  std::vector< coral::Attribute* >::const_iterator isource = rhs.m_data.begin();

  for ( ; idest != m_data.end(); idest++, isource++ )
    (*idest)->fastCopy( **isource );

}



std::ostream&
coral::AttributeList::toOutputStream( std::ostream& os ) const
{
  for ( coral::AttributeList::const_iterator iAttribute = this->begin();
        iAttribute != this->end(); ++iAttribute ) {
    if ( iAttribute != this->begin() ) os << ", ";
    os << "[";
    iAttribute->toOutputStream( os );
    os << "]";
  }
  return os;
}
