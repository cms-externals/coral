#include <ostream>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeException.h"
#include "CoralBase/AttributeSpecification.h"
#include "AttributeData.h"
#include "AttributeDataFactory.h"

namespace coral
{
  /// Exception class for Attribute type mismatches
  class AttributeTypeMismatch : public AttributeException
  {
  public:

    /// Constructor
    AttributeTypeMismatch( const std::string action,
                           const AttributeSpecification* thisSpec,
                           const std::string against,
                           const AttributeSpecification* extSpec )
      : AttributeException( "" )
    {
      setMessage( action + " attribute[\"" + thisSpec->name() + "\"] of type '" + thisSpec->type().name() + "' (" + thisSpec->typeName() + ") " + against + " an attribute of type '" + extSpec->typeName() + "'" );
    }

    /// Constructor
    AttributeTypeMismatch( const std::string action,
                           const AttributeSpecification* thisSpec,
                           const std::string against,
                           const std::type_info& extType )
      : AttributeException( "" )
    {
      std::string extTypeName = "'" + std::string( extType.name() ) + "' (not supported by CORAL)" ;
      try { extTypeName = "'" + AttributeSpecification::typeNameForType( extType ) + "'"; }
      catch( AttributeException& ) {} // throws if type is not supported
      setMessage( action + " attribute[\"" + thisSpec->name() + "\"] of type '" + thisSpec->type().name() + "' (" + thisSpec->typeName() + ") " + against + " an attribute of type " + extTypeName );
    }

    /// Destructor
    virtual ~AttributeTypeMismatch() throw() {}

  };
}


coral::Attribute::Attribute( const coral::AttributeSpecification& specification ) :
  m_specification( &specification ),
  m_data( coral::AttributeDataFactory::factory().create( m_specification->type() ) )
{
}


coral::Attribute::~Attribute()
{
  if ( m_data ) m_data->release();
}

coral::Attribute&
coral::Attribute::operator=( const coral::Attribute& rhs )
{
  if ( !( rhs.m_specification->type() == this->m_specification->type() ) )
    throw AttributeTypeMismatch( "Attempted comparing incompatible types: cannot compare", this->m_specification, "to", rhs.m_specification );
  this->fastCopy( rhs );
  return *this;
}


void
coral::Attribute::fastCopy( const Attribute& rhs )
{
  this->setValueFromAddress( rhs.addressOfData() );
  this->setNull( rhs.isNull() );
}


void
coral::Attribute::setValueFromAddress( const void* externalAddress )
{
  m_data->setValueFromAddress( externalAddress );
}

void
coral::Attribute::copyValueToAddress( void* externalAddress ) const
{
  m_data->copyValueToAddress( externalAddress );
}

void*
coral::Attribute::addressOfData()
{
  return m_data->addressOfData();
}

const void*
coral::Attribute::addressOfData() const
{
  return m_data->addressOfData();
}


void
coral::Attribute::shareData( const Attribute& sourceAttribute )
{
  if ( !( sourceAttribute.m_specification->type() == this->m_specification->type() ) )
    throw AttributeTypeMismatch( "Attempted sharing incompatible types: cannot share", this->m_specification, "with", sourceAttribute.m_specification );
  m_data->shareData( *( sourceAttribute.m_data ) );
}


void
coral::Attribute::bindUnsafely( const void* externalAddress )
{
  m_data->bind( const_cast< void* >( externalAddress ) );
}

void
coral::Attribute::bindUnsafely( void* externalAddress )
{
  m_data->bind( externalAddress );
}


void
coral::Attribute::bindVariable( const std::type_info& type,
                                const void* externalAddress )
{
  if ( ! ( this->m_specification->type() == type ) )
    throw AttributeTypeMismatch( "Attempted binding incompatible types: cannot bind", this->m_specification, "to", type );
  this->bindUnsafely( externalAddress );
}

void
coral::Attribute::bindVariable( const std::type_info& type,
                                void* externalAddress )
{
  if ( ! ( this->m_specification->type() == type ) )
    throw AttributeTypeMismatch( "Attempted binding incompatible types: cannot bind", this->m_specification, "to", type );
  this->bindUnsafely( externalAddress );
}


void
coral::Attribute::setNull( bool isVariableNull )
{
  m_data->setNull( isVariableNull );
}


bool
coral::Attribute::isNull() const
{
  return m_data->isNull();
}


int
coral::Attribute::size() const
{
  return m_data->size();
}



std::ostream&
coral::Attribute::toOutputStream( std::ostream& os,
                                  bool valueOnly) const
{
  if ( ! valueOnly )
    os << m_specification->name() << " (" << m_specification->typeName() << ") : ";
  m_data->toOutputStream( os );
  return os;
}

void
coral::Attribute::setValue( const std::type_info& type,
                            const void* externalAddress )
{
  if ( ! ( this->m_specification->type() == type ) )
    throw AttributeTypeMismatch( "Attempted setting the value from a wrong type: cannot set", this->m_specification, "from", type );
  this->setValueFromAddress( externalAddress );
}


bool
coral::Attribute::operator==( const Attribute& rhs ) const
{
  if (this->m_data->isNull() || rhs.m_data->isNull())
    return this->m_data->isNull() == rhs.m_data->isNull();

  return this->m_specification->type() == rhs.m_specification->type() &&
    *( this->m_data ) == *( rhs.m_data );
}

bool
coral::Attribute::operator!=( const Attribute& rhs ) const
{
  return !(*this == rhs);
}

bool
coral::Attribute::isValidData() const
{
  bool ret = false;
  if(m_data) ret = m_data->isValidData();
  return ret;
}
