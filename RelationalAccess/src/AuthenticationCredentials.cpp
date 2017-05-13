#include "RelationalAccess/AuthenticationCredentials.h"
#include "RelationalAccess/AuthenticationServiceException.h"

coral::AuthenticationCredentials::AuthenticationCredentials( const std::string& implementationName ) :
  m_implementationName( implementationName ),
  m_itemValues(),
  m_itemNames()
{
}


coral::AuthenticationCredentials::~AuthenticationCredentials()
{
}


bool
coral::AuthenticationCredentials::registerItem( const std::string& name,
                                                const std::string& value )
{
  if ( m_itemValues.find( name ) != m_itemValues.end() ) return false;
  m_itemNames.push_back( name );
  m_itemValues.insert( std::make_pair( name, value ) );
  return true;
}


std::string
coral::AuthenticationCredentials::valueForItem( const std::string& itemName ) const
{
  std::map< std::string, std::string >::const_iterator iItem = m_itemValues.find( itemName );
  if ( iItem == m_itemValues.end() )
    throw InvalidCredentialItemException( m_implementationName, itemName );
  return iItem->second;
}


int
coral::AuthenticationCredentials::numberOfItems() const
{
  return static_cast<int>( m_itemNames.size() );
}


std::string
coral::AuthenticationCredentials::itemName( int itemIndex ) const
{
  if ( itemIndex < 0 || itemIndex >= static_cast<int>( m_itemNames.size() ) )
    throw CredentialItemIndexOutOfRangeException( m_implementationName );
  return m_itemNames[ itemIndex ];
}
