#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "AuthenticationCredentialSet.h"
#include "ReplicaSet.h"
#include "LFCReplicaService.h"
#include "LFCReplica.h"
#include "LFCCredentialString.h"
#include "LFCException.h"
#include "LFCReplicaServiceException.h"
#include "LFCPhysicalConnectionString.h"
#include "ReplicaFilter.h"
#include "LFCSession.h"


//#define LOG_DEBUG( msg ) { m_logger << coral::Debug << msg << coral::MessageStream::endmsg; }
#define LOG_DEBUG( msg ) { coral::MessageStream logger( this->name() ); logger << coral::Debug << msg << coral::MessageStream::endmsg; }

#define LOG_WARNING( msg ) { coral::MessageStream logger( this->name() ); logger << coral::Warning << msg << coral::MessageStream::endmsg; }

#define LOG_ERROR( msg ) { coral::MessageStream logger( this->name() ); logger << coral::Error << msg << coral::MessageStream::endmsg; }

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

LFCReplicaService::LFCReplicaService( const std::string& componentName )
  : Service( componentName )
  , m_rootNode( 0 )
  , m_logger( this->name() )
{
  LOG_DEBUG("LFCReplicaService construction...");
  /* default root folder */
  std::string path("");
  std::string name("/database");
  /* check if we have a env variable */
  char* userRootFolder_c = ::getenv("CORAL_LFC_BASEDIR");
  if(userRootFolder_c)
  {
    name.clear();
    name.append(userRootFolder_c);
  }
  /* check path and name */
  LFCNode::validatePath( path, name );
  /* split path from name */
  LOG_DEBUG("start with rootNode [path: " << path << "],[name: " << name << "]");
  /* create new root node */
  m_rootNode = new LFCNode(path, name);
}

//-----------------------------------------------------------------------------

LFCReplicaService::~LFCReplicaService()
{
  LOG_DEBUG("LFCReplicaService destruction...");

  if(m_rootNode) delete m_rootNode;

  clearCredentials();
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::clearCredentials() const
{
  std::map< std::string, coral::AuthenticationCredentialSet* >::iterator i;
  //delete all members
  for ( i = m_credentialMap.begin(); i != m_credentialMap.end(); ++i )
  {
    delete i->second;
  }
  m_credentialMap.clear();
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::appendCredentials(ReplicaDescription& rd, const std::string& filename) const
{
  if(rd.authenticationMechanism().empty())
    return;

  //first treating authentication data
  std::map<std::string, coral::AuthenticationCredentialSet*>::iterator i = m_credentialMap.find( rd.physicalConnectionString() );

  AuthenticationCredentialSet* credForReplica = 0;

  if(i == m_credentialMap.end())
    //not found in the map
  {
    //create a new authentication set
    credForReplica = new AuthenticationCredentialSet( this->name(), rd.physicalConnectionString() );
    //add the set to the map
    m_credentialMap.insert(std::make_pair(rd.physicalConnectionString(), credForReplica));
  }
  else
    credForReplica = i->second;

  if(filename == coral::IAuthenticationCredentials::defaultRole())
  {
    credForReplica->appendCredentialItem(coral::IAuthenticationCredentials::userItem(), rd.userName() );
    credForReplica->appendCredentialItem(coral::IAuthenticationCredentials::passwordItem(), rd.password() );
  }
  else
  {
    credForReplica->appendCredentialItemForRole(coral::IAuthenticationCredentials::userItem(), rd.userName(), filename );
    credForReplica->appendCredentialItemForRole(coral::IAuthenticationCredentials::passwordItem(), rd.password(), filename );
  }
}

//-----------------------------------------------------------------------------

coral::IDatabaseServiceSet*
LFCReplicaService::createServiceSet(LFCNode& node, ReplicaFilter& filter) const
{
  LFCDatabaseServiceSet* sset = new LFCDatabaseServiceSet;
  // Get the vector of files from the cache
  const std::map<std::string, LFCNode*>& nodes = node.getContent();

  std::map<std::string, LFCNode*>::const_iterator j;
  for( j = nodes.begin(); j != nodes.end(); ++j )
  {
    const std::vector<LFCReplica*>& replicas = (j->second)->getReplicas();

    std::vector<LFCReplica*>::const_iterator k;
    for( k = replicas.begin(); k != replicas.end(); ++k )
    {
      ReplicaDescription* rd = new ReplicaDescription(*(*k), j->second->guid() );
      // Add the credentials to the member map
      appendCredentials( *rd , (j->second)->name() );
      // Apply the filter
      if(filter.select(*rd))
        sset->appendReplica( rd );
    }
  }
  return sset;
}

//-----------------------------------------------------------------------------

LFCNode&
LFCReplicaService::lookupNode( const std::string& logicalName ) const
{
  LOG_DEBUG("Lookup for " << logicalName << " in the LFCNode cache");
  /* search for a child folder/file (logical name) in the root folder */
  // Avoiding scanning the folder we can also pretend to have a node, set it only RO
  LFCNode* logical_node = m_rootNode->hasNode( logicalName, true, true, false, true );
  if( !logical_node )
  {
    throw LFCReplicaServiceException("LFC node was not found : " + logicalName, "LFCReplicaService::lookupNode");
  }
  return *logical_node;
}

//-----------------------------------------------------------------------------

coral::IDatabaseServiceSet*
LFCReplicaService::lookup( const std::string& logicalName, coral::AccessMode accessMode, std::string authenticationMechanism) const
{
  ReplicaFilter filter;
  // Set the filter options
  if( !authenticationMechanism.empty() )
    filter.setAuthenticationMechanism( authenticationMechanism );
  // Add default filter options
  filter.setAccessMode( accessMode );
  filter.setStatus( coral::LFCReplicaService::On );
  // Get the cache object and fill the cache if needed
  LFCNode& node = lookupNode( logicalName );
  // Clear all old credentials
  clearCredentials();
  // Create the service set and the credentials and return
  coral::IDatabaseServiceSet* ds = createServiceSet( node, filter );

  return ds;
}

//-----------------------------------------------------------------------------

const coral::IAuthenticationCredentials&
LFCReplicaService::credentials( const std::string& connectionString ) const
{
  // Create a coral message stream for logging
  coral::MessageStream log( this->name() );

  LOG_DEBUG( "Searching connection \""
             << connectionString
             << "\" in the credential map. Total entries="
             << m_credentialMap.size() );
  // Search for the connection string
  std::map<std::string, AuthenticationCredentialSet*>::const_iterator iCred = m_credentialMap.find(connectionString);
  if(iCred == m_credentialMap.end())
  {
    // Not found
    throw coral::UnknownConnectionException("LFCReplicaService", connectionString);
  }
  return iCred->second->credentials();
}

//-----------------------------------------------------------------------------

const coral::IAuthenticationCredentials&
LFCReplicaService::credentials( const std::string& connectionString, const std::string& role ) const
{
  // Create a coral message stream for logging
  coral::MessageStream log( this->name() );

  LOG_DEBUG( "Searching connection \""
             << connectionString
             << "\" in the credential map. Total entries="
             << m_credentialMap.size() );
  // Search for the connection string
  std::map<std::string, AuthenticationCredentialSet*>::const_iterator iCred = m_credentialMap.find(connectionString);
  if(iCred==m_credentialMap.end())
  {
    // Not found
    throw coral::UnknownConnectionException("LFCReplicaService", connectionString);
  }
  return iCred->second->credentials( role );
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::addReplica( const std::string& logicalName, const std::string& theRole, const ReplicaDescription& rd )
{
  // Copy the role object to a local mutable object
  std::string role = theRole;

  if(role.empty())
    role = coral::IAuthenticationCredentials::defaultRole();

  LOG_DEBUG( "Adding replica for logical string=\"" << logicalName << "\", role=\"" << role << "\": pcs=\"" << rd.physicalConnectionString() << "\"." );

  LFCNode* logical_node = m_rootNode->hasNode( logicalName );
  if( !logical_node )
  {
    // Try to create a new node, isDir=false, isWriteable=true, isExecutable=false
    logical_node = m_rootNode->newNode( logicalName, true, true, false );
    // Throw an error if the folder with the logical name doesn't exists
    if( !logical_node )
      throw LFCReplicaServiceException("Folder with the logical name was not created " + logicalName, "LFCReplicaService::addReplica");

    if( !logical_node->valid() )
      throw LFCReplicaServiceException("Folder with the logical name is not valid " + logicalName, "LFCReplicaService::addReplica");

  }

  if( !logical_node->isDir() )
  {
    throw LFCReplicaServiceException(logicalName + " is not a folder", "LFCReplicaService::addReplica");
  }

  if( !logical_node->isWritable() )
  {
    throw LFCReplicaServiceException(logicalName + " has no write permissions", "LFCReplicaService::addReplica");
  }
  // Get the role node which must be a file
  LFCNode* role_node = logical_node->hasNode( role );
  if( !role_node )
  {
    // Try to create a new node, isDir=false, isWriteable=true, isExecutable=false
    role_node = logical_node->newNode( role, false, true, false );

    if( !role_node )
      throw LFCReplicaServiceException("Can't add new role " + role, "LFCReplicaService::addReplica");
  }

  if( role_node->isDir() )
  {
    throw LFCReplicaServiceException(role + " is not a file", "LFCReplicaService::addReplica");
  }
  // Finally create a new replica
  LFCReplica* replica = role_node->newReplica(rd);
  if( !replica )
  {
    throw LFCReplicaServiceException("Can't add replica to LFC with role " + role, "LFCReplicaService::addReplica");
  }

}

//-----------------------------------------------------------------------------

void
LFCReplicaService::importReplicas( const coral::LFCReplicaService::ReplicaSet& replicas )
{
  for(std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >::const_iterator iLcs=replicas.replicaMap().begin();
      iLcs!=replicas.replicaMap().end(); ++iLcs) {
    std::string lcs = iLcs->first;
    for(std::map<std::string,std::multimap<std::string,ReplicaDescription*> >::const_iterator iRole=iLcs->second.begin();
        iRole!=iLcs->second.end(); ++iRole) {
      std::string role = iRole->first;
      for(std::multimap<std::string,ReplicaDescription*>::const_iterator iReplica=iRole->second.begin();
          iReplica!=iRole->second.end(); ++iReplica) {
        this->addReplica(lcs,role,*(iReplica->second));
      }
    }
  }
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::listReplicasAppend(LFCNode& node, ReplicaSet& set, const ReplicaFilter& filter) const
{
  //get all replicas
  const std::vector<LFCReplica*>& replicas = node.getReplicas();

  std::vector<LFCReplica*>::const_iterator k;
  for( k = replicas.begin(); k != replicas.end(); ++k )
  {
    //transform from replica to replica description
    ReplicaDescription* rd = new ReplicaDescription(*(*k), node.guid());
    //execute the filter
    if(filter.select(*rd))
      set.appendReplica( node.path(), node.name(), rd );
  }
}

//-----------------------------------------------------------------------------

ReplicaSet*
LFCReplicaService::listReplicas( const std::string& logicalName, const std::string& role, const ReplicaFilter& filter, bool /*scanChildFolders*/) const
{
  //message
  LOG_DEBUG("Listing replicas for logical string=\"" << logicalName << "\", role=\"" << role << "\"");
  //do some checks
  //get the cache object and fill the cache if needed
  LFCNode& node = lookupNode( logicalName );
  //get all cached files
  const std::map<std::string, LFCNode*>& nodes = node.getContent();
  //create a new replica set
  ReplicaSet* replicaSet = new ReplicaSet;
  // Start a session here
  try
  {
    // For more than 1 entry a session is used
    if( nodes.size() > 1 )
    {
      LFCSession session;
      session.startSession( true );

      std::map<std::string, LFCNode*>::const_iterator j;
      for( j = nodes.begin(); j != nodes.end(); ++j )
      {
        //check if the file fits a role
        if(role.empty())
          listReplicasAppend(*(j->second), *replicaSet, filter);
        else if(role == j->first)
          listReplicasAppend(*(j->second), *replicaSet, filter);
      }
      session.endSession();
    }
    else
    {
      std::map<std::string, LFCNode*>::const_iterator j;
      for( j = nodes.begin(); j != nodes.end(); ++j )
      {
        //check if the file fits a role
        if(role.empty())
          listReplicasAppend(*(j->second), *replicaSet, filter);
        else if(role == j->first)
          listReplicasAppend(*(j->second), *replicaSet, filter);
      }
    }
  }
  catch(...)
  {
    throw;
  }
  return replicaSet;
}

//-----------------------------------------------------------------------------

size_t
LFCReplicaService::deleteReplicas( const std::string& logicalName,
                                   const std::string& role,
                                   const ReplicaFilter& /*filter*/ )
{
  LOG_DEBUG("Deleting replicas for logical string=\"" << logicalName << "\", role=\"" << role);

  if(role.empty())
  {
    LOG_WARNING("The role was not defined");

    return 0;
  }
  // Do the usual checkings
  LFCNode* logical_node = m_rootNode->hasNode( logicalName );
  if( !logical_node )
  {
    throw LFCReplicaServiceException("Folder with the logical name doesn't exists " + logicalName, "LFCReplicaService::deleteReplicas");
  }

  if( !logical_node->isDir() )
  {
    throw LFCReplicaServiceException(logicalName + " is not a folder", "LFCReplicaService::deleteReplicas");
  }

  // Get the role node which must be a file
  LFCNode* role_node = logical_node->hasNode( role );
  if( !role_node )
  {
    throw LFCReplicaServiceException("Role with the name " + role + " doesn't exists", "LFCReplicaService::deleteReplicas");
  }

  if( role_node->isDir() )
  {
    throw LFCReplicaServiceException(role + " is not a file", "LFCReplicaService::deleteReplicas");
  }
  // Workaround to return the amount of deleted replicas
  size_t delreplicas = role_node->getReplicas().size();
  // Clear the replicas from the LFC
  role_node->clearReplicas();

  return delreplicas;
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::setReplicaPassword( const std::string& dbHostName,
                                       const std::string& userName,
                                       const std::string& newPassword )
{
  LOG_DEBUG("Set new password for db account with username=\"" << userName << "\" on host=\"" << dbHostName << "\"");
  LFCReplicaSet lfcReplicas;

  LFCSession::getReplicas( lfcReplicas, "", dbHostName,"");


  if(lfcReplicas.replicas().empty())
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");
    LOG_DEBUG("No replica found for database host \"" << dbHostName << "\".");

    return;
  }

  bool userFound = false;

  std::vector<LFCReplica*>::const_iterator i;
  for( i = lfcReplicas.replicas().begin(); i != lfcReplicas.replicas().end(); ++i)
  {
    ReplicaDescription* replicaDescr = 0;
    try
    {
      replicaDescr = new ReplicaDescription(*(*i), "");
      if(replicaDescr->userName() == userName)
      {
        userFound = true;
        // Delete the existing replica
        LFCSession::deleteReplica( *(*i) );
        // Make a local copy
        LFCReplica replica(*(*i));
        // Create the new password
        replica.fs = LFCCredentialString::credentials( newPassword, replicaDescr->guid() );
        // Add the new replica
        LFCSession::addReplica( replica );
      }
    }
    catch (const ReplicaDescriptionException& exc)
    {
      LOG_ERROR("Failed to process replica with host=\""<<dbHostName<<"\", userName=\""<<userName<<"\": "<< exc.what());
      if(replicaDescr) delete replicaDescr;
      replicaDescr = 0;
    }
  }

  if( !userFound )
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");;
    LOG_DEBUG("No replica found for database user \"" << userName << "\".");
  }
}

//-----------------------------------------------------------------------------

bool
LFCReplicaService::verifyReplicaPassword( const std::string& dbHostName,
                                          const std::string& userName,
                                          const std::string& currentPassword ) const
{
  LOG_DEBUG("Verifying password for db account with username=\"" << userName << "\" on host=\"" << dbHostName << "\"");
  int notSamePassword = 0;
  LFCReplicaSet lfcReplicas;

  LFCSession::getReplicas( lfcReplicas, "", dbHostName,"");


  if(lfcReplicas.replicas().empty())
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");
    LOG_DEBUG("No replica found for database host \"" << dbHostName << "\".");

    return false;
  }

  bool userFound = false;

  std::vector<LFCReplica*>::const_iterator i;
  for( i = lfcReplicas.replicas().begin(); i != lfcReplicas.replicas().end(); ++i)
  {
    ReplicaDescription* replicaDescr = 0;
    try
    {
      replicaDescr = new ReplicaDescription(*(*i), "");
      if(replicaDescr->userName() == userName)
      {
        userFound = true;
        if(replicaDescr->password() != currentPassword)
        {
          ++notSamePassword;
          LOG_WARNING("On replica \"" << (*i)->sfn << "\" the password specified for db_server=\"" << dbHostName << " and user=\"" << userName << "\" is INVALID.");
        }
      }
      delete replicaDescr;
      replicaDescr = 0;
    }
    catch (const ReplicaDescriptionException& exc)
    {
      LOG_ERROR("Failed to process replica with host=\""<<dbHostName<<"\", userName=\""<<userName<<"\": "<< exc.what());
      if(replicaDescr) delete replicaDescr;
      replicaDescr = 0;
    }
  }

  if( !userFound )
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");;
    LOG_DEBUG("No replica found for database user \"" << userName << "\".");
  }

  if( notSamePassword == 0 )
  {
    LOG_DEBUG("All passwords are VALID for username \"" << userName << "\".");
  }
  else
  {
    LOG_ERROR("One or more passwords are INVALID for username \"" << userName << "\".");
  }

  return notSamePassword == 0;
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::setReplicaStatus( const std::string& dbHostName,
                                     const std::string& userName,
                                     ReplicaStatus newStatus)
{
  LOG_DEBUG("Set new status for db account with username=\"" << userName << "\" on host=\"" << dbHostName << "\"");
  LFCReplicaSet lfcReplicas;

  LFCSession::getReplicas( lfcReplicas, "", dbHostName,"");


  if(lfcReplicas.replicas().empty())
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");
    LOG_DEBUG("No replica found for database host \"" << dbHostName << "\".");

    return;
  }

  bool userFound = false;

  std::vector<LFCReplica*>::const_iterator i;
  for( i = lfcReplicas.replicas().begin(); i != lfcReplicas.replicas().end(); ++i)
  {
    ReplicaDescription* replicaDescr = 0;
    try
    {
      replicaDescr = new ReplicaDescription(*(*i), "");
      if(userName.empty() || (replicaDescr->userName() == userName))
      {
        userFound = true;

        LFCSession::setReplicaStatus( *(*i), newStatus == On );
      }
    }
    catch (const ReplicaDescriptionException& exc)
    {
      LOG_ERROR("Failed to process replica with host=\""<<dbHostName<<"\", userName=\""<<userName<<"\": "<< exc.what());
      if(replicaDescr) delete replicaDescr;
      replicaDescr = 0;
    }
  }

  if( !userFound )
  {
    LOG_DEBUG("Nothing to do. Closing transaction.");;
    LOG_DEBUG("No replica found for database user \"" << userName << "\".");
  }
}

//-----------------------------------------------------------------------------

void
LFCReplicaService::setReplicaAccessPermission( const std::string& logicalName,
                                               const std::string& role,
                                               const std::string& groupName,
                                               ReplicaPermission permission)
{
  //do some checks
  //get the cache object and fill the cache if needed
  LFCNode& node = lookupNode( logicalName );

  if( !node.isDir() )
  {
    throw LFCReplicaServiceException(logicalName + " is not a folder", "LFCReplicaService::setReplicaAccessPermission");
  }
  // Get the original permission
  int oldMode = node.getPermissionToGroup(groupName);
  // Ste new permissions
  node.setPermissionToGroup(groupName, permissionMode(permission)|1|oldMode);

  // Get the role node which must be a file
  LFCNode* role_node = node.hasNode( role );
  if( !role_node )
  {
    throw LFCReplicaServiceException("Role with the name " + role + " doesn't exists", "LFCReplicaService::deleteReplicas");
  }

  if( role_node->isDir() )
  {
    throw LFCReplicaServiceException(role + " is not a file", "LFCReplicaService::deleteReplicas");
  }
  // Set here the same permissions
  role_node->setPermissionToGroup(groupName, permissionMode(permission) );
}

//-----------------------------------------------------------------------------
