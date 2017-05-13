#include "CoralCommon/DatabaseServiceSet.h"
#include "CoralCommon/DatabaseServiceDescription.h"
#include "RelationalAccess/LookupServiceException.h"

coral::DatabaseServiceSet::DatabaseServiceSet( const std::string& context ) :
  m_context( context ),
  m_data()
{
}

coral::DatabaseServiceSet::~DatabaseServiceSet()
{
  for ( std::vector< coral::DatabaseServiceDescription *>::iterator iDatabase = m_data.begin();
        iDatabase != m_data.end(); ++iDatabase ) delete *iDatabase;
}


coral::DatabaseServiceSet::DatabaseServiceSet( const coral::DatabaseServiceSet& rhs ) :
  IDatabaseServiceSet(),
  m_context( rhs.m_context ),
  m_data()
{
  for ( std::vector< coral::DatabaseServiceDescription *>::const_iterator iDatabase = rhs.m_data.begin();
        iDatabase != rhs.m_data.end(); ++iDatabase ) {
    m_data.push_back( new coral::DatabaseServiceDescription( **iDatabase ) );
  }
}


coral::DatabaseServiceSet&
coral::DatabaseServiceSet::operator=( const DatabaseServiceSet& rhs )
{
  m_context = rhs.m_context;
  for ( std::vector< coral::DatabaseServiceDescription *>::iterator iDatabase = m_data.begin();
        iDatabase != m_data.end(); ++iDatabase ) delete *iDatabase;
  m_data.clear();
  for ( std::vector< coral::DatabaseServiceDescription *>::const_iterator iDatabase = rhs.m_data.begin();
        iDatabase != rhs.m_data.end(); ++iDatabase ) {
    m_data.push_back( new coral::DatabaseServiceDescription( **iDatabase ) );
  }
  return *this;
}


coral::DatabaseServiceDescription&
coral::DatabaseServiceSet::appendReplica( const std::string& connectionName,
                                          const std::string& authenticationMechanism,
                                          AccessMode accessMode )
{
  m_data.push_back( new coral::DatabaseServiceDescription( connectionName,
                                                           authenticationMechanism,
                                                           accessMode ) );
  return *m_data.back();
}


int
coral::DatabaseServiceSet::numberOfReplicas() const
{
  return static_cast<int>( m_data.size() );
}


const coral::IDatabaseServiceDescription&
coral::DatabaseServiceSet::replica( int index ) const
{
  if ( index < 0 || index >= static_cast<int>( m_data.size() ) )
    throw coral::InvalidReplicaIdentifierException( m_context );
  return *( m_data[index] );
}
