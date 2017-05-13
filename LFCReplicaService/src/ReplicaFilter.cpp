#include "ReplicaFilter.h"

#include "CoralBase/MessageStream.h"

#define LOG_DEBUG( msg ) { coral::MessageStream logger( "ReplicaFilter" ); logger << coral::Debug << msg << coral::MessageStream::endmsg; }

using namespace coral::LFCReplicaService;

ReplicaFilter::ReplicaFilter()
  : m_physicalConnectionStringFilter( std::string("") , false)
  , m_authenticationMechanismFilter( std::string("") , false)
  , m_userNameFilter(std::string("") , false)
  , m_passwordFilter(std::string("") , false)
  , m_serverNameFilter(std::string("") , false)
  , m_statusFilter(coral::LFCReplicaService::On , false)
  , m_accessModeFilter(coral::Update , false)
  , m_doFilter( false )
{
}

ReplicaFilter::ReplicaFilter(coral::AccessMode& accessMode, std::string& authMech, ReplicaStatus status)
  : m_physicalConnectionStringFilter( "" , false)
  , m_authenticationMechanismFilter( authMech , true)
  , m_userNameFilter("" , false)
  , m_passwordFilter("" , false)
  , m_serverNameFilter("" , false)
  , m_statusFilter(status , true)
  , m_accessModeFilter(accessMode , true)
  , m_doFilter( true )
{
}

ReplicaFilter::ReplicaFilter( const ReplicaFilter& rhs)
  : m_physicalConnectionStringFilter(rhs.m_physicalConnectionStringFilter)
  , m_authenticationMechanismFilter(rhs.m_authenticationMechanismFilter)
  , m_userNameFilter(rhs.m_userNameFilter)
  , m_passwordFilter(rhs.m_passwordFilter)
  , m_serverNameFilter(rhs.m_serverNameFilter)
  , m_statusFilter(rhs.m_statusFilter)
  , m_accessModeFilter(rhs.m_accessModeFilter)
  , m_doFilter(false)
{
}

bool
ReplicaFilter::select(const ReplicaDescription& replica) const
{
  bool ok = true;
  if(m_doFilter)
  {
    if( m_physicalConnectionStringFilter.second )
    {
      ok = ok && (m_physicalConnectionStringFilter.first == replica.physicalConnectionString());
      LOG_DEBUG("apply filter physicalConnectionString " << ok);
    }
    if( m_userNameFilter.second )
    {
      ok = ok && (m_userNameFilter.first == replica.userName());
      LOG_DEBUG("apply filter userName " << ok);
    }
    if( m_serverNameFilter.second )
    {
      ok = ok && (m_serverNameFilter.first == replica.serverName());
      LOG_DEBUG("apply filter host " << ok);
    }
    if( m_statusFilter.second )
    {
      ok = ok && (m_statusFilter.first == replica.status());
      LOG_DEBUG("apply filter status " << ok);
    }
    if( m_accessModeFilter.second )
    {
      ok = ok && (m_accessModeFilter.first == replica.accessMode());
      LOG_DEBUG("apply filter accessMode " << ok);
    }
    if( m_authenticationMechanismFilter.second )
    {
      ok = ok && (m_authenticationMechanismFilter.first == replica.authenticationMechanism());
      LOG_DEBUG("apply filter authenticationMechanism [" << ok << "] [" << m_authenticationMechanismFilter.first << "] ? [" << replica.authenticationMechanism() << "]");
    }
  }
  return ok;
}
