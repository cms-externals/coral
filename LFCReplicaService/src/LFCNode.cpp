#include "LFCNode.h"
#include <sys/stat.h> //mode_t in lfc_api.h
#include <lfc_api.h>
#include <serrno.h>

#include <string>
#include <cstring>

#include "LFCSession.h"
#include "LFCException.h"
#include "ReplicaSet.h"
#include "Guid.h"

#include "CoralBase/MessageStream.h"

#define LOG_ID "LFCNode [path:" << m_path << "],[name: " << m_name << "] "

#define DEBUG( msg ) { coral::MessageStream logger( "LFCNode" ); logger << coral::Debug << msg << coral::MessageStream::endmsg; }

#define LOG_DEBUG( msg ) { coral::MessageStream logger( "LFCNode" ); logger << coral::Debug << LOG_ID << msg << coral::MessageStream::endmsg; }

#define LOG_WARNING( msg ) { coral::MessageStream logger( "LFCNode" ); logger << coral::Warning << LOG_ID << msg << coral::MessageStream::endmsg; }


namespace {

  class LFCScopedOpenDir
  {

  public:

    LFCScopedOpenDir(const std::string& path)
      : m_dirptr( 0 )
      , m_dirrep( 0 )
    {
      coral::LFCReplicaService::LFCConnection connection;

      // Try to open a LFC folder
      m_dirptr = lfc_opendirg(path.c_str(), "");

      while( connection.retry("lfc_opendir", m_dirptr) )
      {
        // Try to open a LFC folder
        m_dirptr = lfc_opendirg(path.c_str(), "");
      }

      DEBUG("opendir successful on path " << path);
    }

    ~LFCScopedOpenDir()
    {
      lfc_closedir( m_dirptr );
      DEBUG("closedir successful");
    }

    bool next()
    {
      m_dirrep = lfc_readdirxr(m_dirptr, (char*)"");

      return m_dirrep != 0;
    }

    struct lfc_direnrep* current()
    {
      return m_dirrep;
    }

  private:

    lfc_DIR* m_dirptr;

    struct lfc_direnrep* m_dirrep;

  };

}

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

void
LFCNode::validatePath( std::string& path, std::string& name )
{
  coral::MessageStream logger( "LFCNode" );
  logger << coral::Debug << "validate path,name [" << path  << "],[" << name << "]" << coral::MessageStream::endmsg;

  // Get rid of '/' at the end of path and name
  if( path[path.size() - 1] == '/' )
    path = path.substr( 0, path.size() - 1 );
  if( name[name.size() - 1] == '/' )
    name = name.substr( 0, name.size() - 1 );

  std::string::size_type pos_found = name.find_last_of( "/" );
  if( pos_found != std::string::npos )
  {
    // Extract the path
    std::string path2 = name.substr( 0, pos_found);
    // Extract the name
    std::string name2 = name.substr( pos_found + 1 );
    // Copy the correct name
    name = name2;
    // Copy the correct folder
    if( path2[0] == '/' )
    {
      // Override the current path in the case of new absolute path
      path = path2;
    }
    else
    {
      // Just append the new found path
      path.append(path2);
    }
  }
}

//-----------------------------------------------------------------------------

LFCNode::LFCNode( const std::string& path, const std::string& name )
  : m_name( name )
  , m_path( path )
  , m_lfclookup_self( false )
  , m_lfclookup_content( false )
  , m_lfclookup_replicas( false )
  , m_valid( true ) //must be validated by calling lookupSelf
  , m_isdir( false )
  , m_readOk( false )
  , m_writeOk( false )
  , m_executeOk( false )
  , m_guid("")
{
  validatePath( m_path, m_name );
}

//-----------------------------------------------------------------------------

LFCNode::LFCNode( const std::string& path, const std::string& name, bool isDir, bool readOk, bool writeOk, bool exeOk, const char* guid )
  : m_name( name )
  , m_path( path )
  , m_lfclookup_self( true )
  , m_lfclookup_content( false )
  , m_lfclookup_replicas( false )
  , m_valid( true )
  , m_isdir( isDir )
  , m_readOk( readOk )
  , m_writeOk( writeOk )
  , m_executeOk( exeOk )
  , m_guid( guid )
{
}

//-----------------------------------------------------------------------------

LFCNode::~LFCNode()
{
  // Release the replica entries for this node
  std::vector<LFCReplica*>::iterator i;
  for( i = m_replicas.begin(); i != m_replicas.end(); ++i )
  {
    delete (*i);
  }
  m_replicas.clear();
  // Release all child nodes
  std::map<std::string, LFCNode*>::iterator j;
  for( j = m_content.begin(); j != m_content.end(); ++j )
  {
    delete j->second;
  }
  m_content.clear();
}

//-----------------------------------------------------------------------------

std::string
LFCNode::absolutePath()
{
  std::string ret;

  ret.append(m_path);
  ret.append("/");
  ret.append(m_name);

  return ret;
}

//-----------------------------------------------------------------------------

void
LFCNode::appendReplica( LFCReplica* replica )
{
  m_replicas.push_back( replica );

  LOG_DEBUG("replica added [host:" << replica->host << "]");
}

//-----------------------------------------------------------------------------

const std::vector<LFCReplica*>&
LFCNode::getReplicas()
{
  if( !m_lfclookup_replicas )
  {
    lookupReplicas();
  }

  return m_replicas;
}

//-----------------------------------------------------------------------------

const std::map<std::string, LFCNode*>&
LFCNode::getContent()
{
  if( !m_lfclookup_self )
  {
    lookupSelf();

    if( m_valid )
      lookupContent();
  }
  else if( (!m_lfclookup_content) && m_valid )
  {
    lookupContent();
  }
  return m_content;
}

//-----------------------------------------------------------------------------

void
LFCNode::lookupSelf()
{
  m_lfclookup_self = true;

  LOG_DEBUG("lookup on itself");

  struct lfc_filestatg statg;

  int res = lfc_statg(absolutePath().c_str(), 0, &statg);

  if( res == 0 )
  {
    LOG_DEBUG("exists");
    /* file/folder exists */
    m_valid = true;
    /* is folder/file */
    m_isdir = S_ISDIR(statg.filemode);
    /* extract the file permissions */
    m_readOk = (statg.filemode & S_IRUSR) == S_IRUSR;
    m_writeOk = (statg.filemode & S_IWUSR) == S_IWUSR;
    m_executeOk = (statg.filemode & S_IXUSR) == S_IXUSR;
    /* copy the global unique id */
    m_guid = statg.guid;
  }
}

//-----------------------------------------------------------------------------

void
LFCNode::lookupContent()
{
  // Is it a directory
  if( !m_isdir ) return;
  // Have we read access
  if( !m_readOk ) return;
  // Have we execution access
  if( !m_executeOk ) return;

  m_lfclookup_content = true;
  // Open a try block to use the C++ stack unwinding in exception case
  try
  {
    std::string node_path = absolutePath();

    LFCScopedOpenDir opendir( node_path );

    while(opendir.next())
    {
      struct lfc_direnrep* read_pt = opendir.current();

      if( S_ISDIR(read_pt->filemode) )
      {
        // Create a new folder node
        LFCNode* node = new LFCNode( node_path
                                     , read_pt->d_name
                                     , true
                                     , (read_pt->filemode & S_IRUSR) == S_IRUSR
                                     , (read_pt->filemode & S_IWUSR) == S_IWUSR
                                     , (read_pt->filemode & S_IXUSR) == S_IXUSR
                                     , read_pt->guid
                                     );
        /* add the node to the map */
        m_content.insert( std::pair<std::string, LFCNode*>(read_pt->d_name, node) );
        LOG_DEBUG("found folder node [name:" << read_pt->d_name << "]");
      }
      else
      {
        // Create a new file node
        LFCNode* node = new LFCNode( node_path
                                     , read_pt->d_name
                                     , false
                                     , (read_pt->filemode & S_IRUSR) == S_IRUSR
                                     , (read_pt->filemode & S_IWUSR) == S_IWUSR
                                     , (read_pt->filemode & S_IXUSR) == S_IXUSR
                                     , read_pt->guid
                                     );
        /* add the node to the map */
        m_content.insert( std::pair<std::string, LFCNode*>(read_pt->d_name, node) );
        LOG_DEBUG("found file node [name:" << read_pt->d_name << "]");
      }
    }
  }
  // catch all exceptions
  catch( ... )
  {
    // rethrow it
    throw;
  }
}

//-----------------------------------------------------------------------------

void
LFCNode::lookupReplicas()
{
  m_lfclookup_replicas = true;

  lfc_filereplica* replicas = 0;

  try
  {

    if( m_readOk && (!m_isdir) )
    {
      // Integer for the amount entries
      int entries = 0;
      // Do the LFC call
      coral::LFCReplicaService::LFCConnection connection;

      // Try to get replicas from LFC
      int ret = lfc_getreplica( absolutePath().c_str(), 0, 0, &entries, &replicas );

      while( connection.retry("lfc_opendir", ret == 0) )
      {
        // Try again
        ret = lfc_getreplica( absolutePath().c_str(), 0, 0, &entries, &replicas );
      }

      lfc_filereplica* replica = replicas;
      for( int i = 0; i < entries; i++ )
      {
        appendReplica( new LFCReplica(replica) );
        //node->appendReplica( new LFCReplicaInfo(read_pt->rep[i]) );
        // Increase to the next replica of the array
        replica++;
      }
      // Release memory
      free( replicas );
      replicas = 0;
    }
  }
  catch( ... )
  {
    if( replicas )
      free( replicas );
    // rethrow it
    throw;
  }
}

//-----------------------------------------------------------------------------

LFCNode*
LFCNode::newNode( const std::string& name, bool isDir, bool /*isWritable*/, bool /*isExecutable*/ )
{
  if( !m_lfclookup_self )
  {
    lookupSelf();

    if( m_valid )
      lookupContent();

  }
  LFCNode* node = 0;

  if( m_isdir )
  {
    std::string path(absolutePath());
    path.append( "/" );
    path.append( name );

    //lfc_starttrans( ::getenv("LFC_HOST"), "CORAL" );

    int res = -1;
    if( isDir )
    {
      // Create node as dir
      res = lfc_mkdir(path.c_str(), 0750);
    }
    else
    {
      Guid newGuid;
      Guid::create(newGuid);
      // Create node as file
      res = lfc_creatg(path.c_str(), newGuid.toString().c_str(), 0700);
    }
    // OK?
    if(res == 0)
    {
      // Create the new virtual node
      node = new LFCNode(absolutePath(), name);
      // Add the node
      m_content.insert( std::pair<std::string, LFCNode*>(name, node) );
      LOG_DEBUG("new node was created and added [name:" << name << "]");
    }
    else
    {
      LOG_WARNING("Can't create node [" << name << "]: " << sstrerror(serrno) << " Arguments: " << path.c_str() );
    }
  }
  else
  {
    LOG_WARNING("Node is not a directory, can't create a new node in it");
  }
  // Return the node, if ZERO error happen
  return node;
}

//-----------------------------------------------------------------------------

LFCReplica*
LFCNode::newReplica(const ReplicaDescription& rd)
{
  if( !m_lfclookup_self )
  {
    lookupSelf();

    if( m_valid )
      lookupContent();
  }
  else if( (!m_lfclookup_content) && m_valid )
  {
    lookupContent();
  }

  LFCReplica* replica = 0;

  // Must be a file
  if( !m_isdir )
  {
    replica = new LFCReplica(rd, m_guid);

    int res = lfc_addreplica(m_guid.c_str(),
                             NULL,
                             replica->host.c_str(),
                             replica->sfn.c_str(),
                             replica->status ? '1' : '0',
                             replica->f_type,
                             replica->poolname.c_str(),
                             replica->fs.c_str());

    // If res != 0 error ocoured
    if( res == 0 )
    {
      this->appendReplica( replica );
    }
    else
    {
      LOG_WARNING("Can't add replica: " << sstrerror(serrno) );
      LOG_WARNING("Aguments [host:" << replica->host << "],[sfn:" << replica->sfn << "],[status:" << replica->status << "],[f_type: " << replica->f_type << "],[poolname:" << replica->poolname << "],[fs:" << replica->fs );

      delete replica;
      replica = 0;
    }
  }
  else
  {
    LOG_WARNING("Node is a directory, can't create a replica");
  }

  return replica;
}

//-----------------------------------------------------------------------------

void
LFCNode::clearReplicas()
{
  // Release the replica entries for this node
  std::vector<LFCReplica*>::iterator i;
  for( i = m_replicas.begin(); i != m_replicas.end(); ++i )
  {
    LFCSession::deleteReplica( *(*i) );
    delete (*i);
  }
  m_replicas.clear();
}

//-----------------------------------------------------------------------------

LFCNode*
LFCNode::hasNode( const std::string& name )
{
  const std::map<std::string, LFCNode*>& content = this->getContent();

  std::map<std::string, LFCNode*>::const_iterator i = content.find( name );
  if( i == content.end() )
    return 0;
  else
    return i->second;
}

//-----------------------------------------------------------------------------

LFCNode*
LFCNode::hasNode( const std::string& name, bool isDir, bool readOk, bool writeOk, bool exeOk )
{
  std::map<std::string, LFCNode*>::const_iterator i = m_content.find( name );
  if( i == m_content.end() )
  {
    LFCNode* node = new LFCNode( absolutePath(), name, isDir, readOk, writeOk, exeOk, "" );

    m_content.insert(std::pair<std::string, LFCNode*>(name, node));

    return node;
  }
  else
    return i->second;
}

//-----------------------------------------------------------------------------

bool
LFCNode::valid()
{
  if( !m_lfclookup_self )
  {
    this->lookupSelf();

    if( m_valid )
      lookupContent();
  }
  return m_valid;
}

//-----------------------------------------------------------------------------

void
LFCNode::setPermissionToGroup(const std::string& groupName, int mode)
{
  std::string path = absolutePath();

  gid_t groupId = 0;
  std::string gName(groupName);
  if (lfc_getgrpbynam(const_cast<char*>(gName.c_str()),&groupId) < 0)
  {
    throw LFCException("Cannot resolve group Id.","lfc_getgrpbyname");
  }
  int nentries = 0;
  nentries = lfc_getacl(path.c_str(),0,NULL);
  if (nentries < 0)
  {
    throw LFCException("Cannot get the number of acl entries for the specified file.","lfc_getacl");
  }
  lfc_acl* aclEntries = static_cast<lfc_acl*>(::calloc(nentries+2,sizeof(lfc_acl)));
  if(nentries >0)
  {
    if (lfc_getacl(path.c_str(),nentries,aclEntries) < 0)
    {
      ::free(aclEntries);
      throw LFCException("Cannot get the acl entries for the specified file.","lfc_setacl");
    }
  }
  lfc_acl*  aclForGroup = 0;
  lfc_acl* aclMask = 0;
  for(int i=0; i<nentries; i++)
  {
    if((aclEntries[i].a_type == CNS_ACL_GROUP) && (aclEntries[i].a_id==int(groupId)))
    {
      aclForGroup = &aclEntries[i];
      break;
    }
  }
  for(int i=0; i<nentries; i++)
  {
    if(int(aclEntries[i].a_type) == CNS_ACL_MASK)
    {
      aclMask = &aclEntries[i];
      break;
    }
  }
  if(!aclForGroup)
  {
    aclForGroup = &aclEntries[nentries];
    ++nentries;
  }
  if(!aclMask)
  {
    aclMask = &aclEntries[nentries];
    aclMask->a_perm = 0;
    ++nentries;
  }
  aclForGroup->a_type = CNS_ACL_GROUP;
  aclForGroup->a_id = groupId;
  aclForGroup->a_perm = mode;
  aclMask->a_type = CNS_ACL_MASK;
  aclMask->a_id = 0;
  aclMask->a_perm |= 5;
  if (lfc_setacl(path.c_str(),nentries, aclEntries) < 0)
  {
    ::free(aclEntries);
    throw LFCException("Cannot set acl to the specified file.","lfc_setacl");
  }
  ::free(aclEntries);
}

//-----------------------------------------------------------------------------

int
LFCNode::getPermissionToGroup(const std::string& groupName)
{
  std::string path = absolutePath();

  gid_t groupId = 0;
  std::string gName(groupName);

  if (lfc_getgrpbynam(const_cast<char*>(gName.c_str()), &groupId) < 0)
  {
    throw LFCException("Cannot resolve group Id.","lfc_getgrpbyname");
  }
  int nentries = 0;
  nentries = lfc_getacl(path.c_str(),0,NULL);
  if (nentries < 0)
  {
    throw LFCException("Cannot get the number of acl entries for the specified file.","lfc_getacl");
  }
  int perm = 0;
  if(nentries > 0)
  {
    lfc_acl* aclEntries = static_cast<lfc_acl*>(::calloc(nentries,sizeof(lfc_acl)));
    if (lfc_getacl(path.c_str(),nentries,aclEntries) < 0)
    {
      ::free(aclEntries);
      throw LFCException("Cannot get the acl entries for the specified file.","lfc_setacl");
    }
    for(int i=0; i<nentries; i++)
    {
      if((aclEntries[i].a_type == CNS_ACL_GROUP) && (aclEntries[i].a_id==int(groupId)))
      {
        perm |= aclEntries[i].a_perm;
      }
    }
    ::free(aclEntries);
  }
  return perm;
}

//-----------------------------------------------------------------------------
