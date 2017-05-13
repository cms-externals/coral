#include "LFCDatabaseServiceSet.h"
#include "RelationalAccess/LookupServiceException.h"

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

LFCDatabaseServiceDescription::LFCDatabaseServiceDescription( ReplicaDescription* replica )
  : m_replica( replica )
{
}

//-----------------------------------------------------------------------------

LFCDatabaseServiceDescription::~LFCDatabaseServiceDescription()
{
  delete m_replica;
}

//-----------------------------------------------------------------------------

std::string
LFCDatabaseServiceDescription::serviceParameter(const std::string& parameterName) const
{
  if(parameterName==coral::IDatabaseServiceDescription::serverNameParam()) {
    return m_replica->serverName();
  } else if (parameterName==coral::IDatabaseServiceDescription::serverStatusParam()) {
    std::string stat = coral::IDatabaseServiceDescription::serverStatusOnline();
    if(!m_replica->status()) stat = coral::IDatabaseServiceDescription::serverStatusOffline();
    return stat;
  } else {
    return "";
  }
}

//-----------------------------------------------------------------------------

LFCDatabaseServiceSet::LFCDatabaseServiceSet()
  : m_sorted( false )
  , m_replicas()
  , m_replicaDescriptions()
{
}

//-----------------------------------------------------------------------------

LFCDatabaseServiceSet::~LFCDatabaseServiceSet()
{
  std::map<std::string, LFCDatabaseServiceDescription*>::const_iterator i;

  for(i = m_replicaDescriptions.begin(); i != m_replicaDescriptions.end(); ++i)
  {
    delete i->second;
  }
}

//-----------------------------------------------------------------------------

void
coral::LFCReplicaService::LFCDatabaseServiceSet::appendReplica( ReplicaDescription* replica )
{
  if(replica)
  {
    if(m_replicaDescriptions.find(replica->physicalConnectionString()) == m_replicaDescriptions.end())
    {
      m_replicaDescriptions.insert(std::make_pair(replica->physicalConnectionString(), new LFCDatabaseServiceDescription(replica)));
      m_sorted = false;
    }
    else
      delete replica;
  }
}

//-----------------------------------------------------------------------------

int coral::LFCReplicaService::LFCDatabaseServiceSet::numberOfReplicas() const
{
  return static_cast<int>( m_replicaDescriptions.size());
}

//-----------------------------------------------------------------------------

const coral::IDatabaseServiceDescription&
LFCDatabaseServiceSet::replica( int index ) const
{
  if(index < 0 || index >= static_cast<int>( m_replicaDescriptions.size()) )
  {
    throw coral::InvalidReplicaIdentifierException( "LFCReplicaService" );
  }
  if(!m_sorted)
  {
    m_replicas.clear();
    for(std::map<std::string,LFCDatabaseServiceDescription*>::const_iterator iR=m_replicaDescriptions.begin();
        iR!=m_replicaDescriptions.end();
        ++iR) {
      m_replicas.push_back(iR->second);
    }
    m_sorted = true;
  }
  return *(m_replicas[index]);
}

//-----------------------------------------------------------------------------
