#include "LFCSession.h"
#include "LFCReplica.h"
#include "ReplicaSet.h"

#include <sys/stat.h> //mode_t in lfc_api.h
#include <lfc_api.h>
#include <cstdlib>
#include <cstring>
#include <serrno.h>

#include "LFCException.h"

#include "CoralBase/MessageStream.h"
#include "CoralCommon/Utilities.h"

#define LOG_DEBUG( msg ) { coral::MessageStream logger( "LFCSession" ); logger << coral::Debug << msg << coral::MessageStream::endmsg; }

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

void
LFCSession::getReplicas( LFCReplicaSet& replicas,
                         const std::string& poolname,
                         const std::string& host,
                         const std::string& fs )
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");

  struct lfc_filereplica *lfcFileReplica;
  lfc_list lfcList;

  int flags = CNS_LIST_BEGIN;
  const char* poolname_c = ( poolname.empty() ? NULL : poolname.c_str() );
  const char* server_c = ( host.empty() ? NULL : host.c_str() );
  const char* fs_c = ( fs.empty() ? NULL : fs.c_str() );
  //lfc_listreplicax( const char* poolname, const char* server, const char* fs, int flags, lfc_list* listp)
  //lfc_getreplicax( const char* path, const char* guid, const char* se, int* nbentries, struct lfc_filereplicax** rep_entries)


  while((lfcFileReplica = lfc_listreplicax(poolname_c,
                                           server_c,
                                           fs_c,
                                           flags,
                                           &lfcList)) != NULL) {
    LFCReplica* replica = new LFCReplica( lfcFileReplica );
    replicas.replicas().push_back(replica);

    flags=CNS_LIST_CONTINUE;
  }

  // Final call to terminate list and free the resources. Refer lfc_listreplica man pages.
  (void) lfc_listreplicax(poolname_c,
                          server_c,
                          fs_c,
                          CNS_LIST_END,
                          &lfcList);
}

//-----------------------------------------------------------------------------

void
LFCSession::deleteReplica( LFCReplica& replica )
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");

  lfc_fileid fid;
  ::strcpy(fid.server, replica.host.c_str() );
  fid.fileid = replica.fileid;

  LOG_DEBUG("Try to delete replica " << replica.fileid);

  if (lfc_delreplica(NULL, &fid, replica.sfn.c_str()) < 0 )
  {
    throw LFCException("Could Not Delete the Replica: " + replica.sfn, "lfc_delreplica");
  }
}

//-----------------------------------------------------------------------------

void
LFCSession::addReplica( LFCReplica& replica )
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");

  lfc_fileid fid;
  ::strcpy(fid.server, replica.host.c_str() );
  fid.fileid = replica.fileid;
  if (lfc_addreplica(NULL,
                     &fid,
                     replica.host.c_str(),
                     replica.sfn.c_str(),
                     replica.status ? '1' : '0',
                     replica.f_type,
                     replica.poolname.c_str(),
                     replica.fs.c_str()) < 0 )
  {
    throw LFCException("Could not add the replica.","lfc_addreplica");
  }
}

//-----------------------------------------------------------------------------

void
LFCSession::setReplicaStatus( LFCReplica& replica, bool status )
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");

  if(lfc_setrstatus(replica.sfn.c_str(), status ? '1' : '0' ) < 0)
  {
    throw LFCException("Could not change status of replica","lfc_setrstatus");
  }
}

//-----------------------------------------------------------------------------

LFCSession::LFCSession()
  : m_openctn( 0 )
  , m_started( false )
  , m_retry_period( 1 ) // set to 1 second
  , m_timeout( 3 ) // set to 3 seconds
{
  /* get the LFC host from the env variable */
  char* lfcHost = ::getenv("LFC_HOST");
  if( lfcHost )
  {
    m_hosts.push_back(lfcHost);
  }
  else
  {
    /* try if we have several hosts defined */
    char* lfcHosts = ::getenv("CORAL_LFC_HOSTS");
    if( !lfcHosts )
    {
      throw LFCException("LFC host has not been defined");
    }
    /* parse the hosts from the string */
    std::string input(lfcHosts);

    std::string::size_type last_pos = 0;
    std::string::size_type pos = 0;

    while(true)
    {
      pos = input.find_first_of(';', last_pos);
      if( pos == std::string::npos )
      {
        m_hosts.push_back(input.substr(last_pos));
        break;
      }
      else
      {
        m_hosts.push_back(input.substr(last_pos, pos - last_pos));
        last_pos = pos + 1;
      }
    }
  }
  // Get the retry period if set
  char* retry = ::getenv("CORAL_LFC_RETRY_PERIOD");
  if( retry )
    m_retry_period = atoi(retry);
  // Get the timeout period if set
  char* timeout = ::getenv("CORAL_LFC_RETRY_TIMEOUT");
  if( timeout )
    m_timeout = atoi(timeout);

  if( m_retry_period > m_timeout )
    m_retry_period = m_timeout;

  std::vector< std::string >::iterator i;
  for( i = m_hosts.begin(); i < m_hosts.end(); ++i )
  {
    LOG_DEBUG("LFC Host used: " << *i);
  }
}

//-----------------------------------------------------------------------------

LFCSession::~LFCSession()
{
  if( m_started )
  {
    /* close LFC session */
    lfc_endsess();
    LOG_DEBUG("LFC session terminated");
    m_started = false;
  }
}

//-----------------------------------------------------------------------------

bool
LFCSession::tryConnect(const char* host)
{
  time_t startime = time(0);

  size_t diff = 0;
  while( diff <  m_timeout )
  {
    LOG_DEBUG("Try to connect to LFC host " << host);
    if( lfc_startsess((char*)host, (char*)"") == 0 )
    {
      // Setup the environment variable for the correct LFC host
#ifdef _WIN32
      std::string envc("LFC_HOST=");
      envc.append(host);
      // FIXME replace with setenv
      ::_putenv( const_cast<char*>(envc.c_str()) );
#else
      ::setenv( "LFC_HOST", host, 1 );
#endif

      LOG_DEBUG("env set to LFC_HOST=" << ::getenv("LFC_HOST"));

      return true;
    }
    // Sleep the retry period
    coral::sleepSeconds( m_retry_period );
    // Calculate the passed time difference
    diff = time(0) - startime;
  }
  return false;
}

//-----------------------------------------------------------------------------

void
LFCSession::startSession(bool force)
{
  if( force && (!m_started) )
  {
    std::vector< std::string >::iterator i;
    for( i = m_hosts.begin(); i < m_hosts.end(); ++i )
    {
      const char* host = i->c_str();
      if( tryConnect(host) )
      {
        LOG_DEBUG("LFC session to host " << host << " successful started");
        m_started = true;
        m_openctn++;
        return;
      }
    }
    if( !m_started )
    {
      throw LFCException("Can't create LFC session");
    }
  }
  m_openctn++;
}

//-----------------------------------------------------------------------------

void
LFCSession::endSession()
{
  m_openctn--;
  if( (m_openctn == 0) && m_started )
  {
    /* close LFC session */
    lfc_endsess();
    LOG_DEBUG("LFC session to host terminated");
    m_started = false;
  }
}

//-----------------------------------------------------------------------------

LFCConnection::LFCConnection()
  : m_retry_period( 1 ) // set to 1 second
  , m_timeout( 3 ) // set to 3 seconds
  , m_startime( time(0) )
  , m_starthost( 0 )
{
  /* get the LFC host from the env variable */
  char* lfcHost = ::getenv("LFC_HOST");
  if( lfcHost )
  {
    m_hosts.push_back(lfcHost);
  }
  else
  {
    /* try if we have several hosts defined */
    char* lfcHosts = ::getenv("CORAL_LFC_HOSTS");
    if( !lfcHosts )
    {
      throw LFCException("LFC host has not been defined");
    }
    /* parse the hosts from the string */
    std::string input(lfcHosts);

    std::string::size_type last_pos = 0;
    std::string::size_type pos = 0;

    while(true)
    {
      pos = input.find_first_of(';', last_pos);
      if( pos == std::string::npos )
      {
        m_hosts.push_back(input.substr(last_pos));
        break;
      }
      else
      {
        m_hosts.push_back(input.substr(last_pos, pos - last_pos));
        last_pos = pos + 1;
      }
    }
  }
  // Get the retry period if set
  char* retry = ::getenv("CORAL_LFC_RETRY_PERIOD");
  if( retry )
    m_retry_period = atoi(retry);
  // Get the timeout period if set
  char* timeout = ::getenv("CORAL_LFC_RETRY_TIMEOUT");
  if( timeout )
    m_timeout = atoi(timeout);

  if( m_retry_period > m_timeout )
    m_retry_period = m_timeout;

  std::vector< std::string >::iterator i;
  for( i = m_hosts.begin(); i < m_hosts.end(); ++i )
  {
    LOG_DEBUG("LFC Host used: " << *i);
  }
  setCurrentHost();

  std::string& host = m_hosts[m_starthost];

  LOG_DEBUG("Try to connect to LFC host " << host << " with first attempt");
}

//-----------------------------------------------------------------------------

LFCConnection::~LFCConnection()
{
}

//-----------------------------------------------------------------------------

void
LFCConnection::setCurrentHost()
{
  std::string& host = m_hosts[m_starthost];
  // Setup the environment variable for the correct LFC host
#ifdef _WIN32
  std::string envc("LFC_HOST=");
  envc.append(host);
  // FIXME replace with setenv
  ::_putenv( const_cast<char*>(envc.c_str()) );
#else
  ::setenv( "LFC_HOST", host.c_str(), 1 );
#endif

  LOG_DEBUG("env set to LFC_HOST=" << ::getenv("LFC_HOST"));
}

//-----------------------------------------------------------------------------

bool
LFCConnection::retry(const std::string& callname, bool status)
{
  int error = serrno;

  if( status )
  {
    return false;
  }
  // Filter only some errors
  // Host not found
  // Communication error
  else if( error == SENOSHOST || error == SECOMERR )
  {
    size_t diff = time(0) - m_startime;
    if( diff <  m_timeout )
    {
      // Sleep the retry period
      coral::sleepSeconds( m_retry_period );
    }
    else
    {
      if( m_starthost < ( m_hosts.size() - 1 ) )
      {
        // Use the next host
        m_starthost++;
        // Ste environment variables
        setCurrentHost();
        // Reset timout
        m_startime = time(0);
      }
      else
      {
        // Timeout on all hosts
        throw coral::LFCReplicaService::LFCException("Can't establish LFC connection", callname);
      }
    }
  }
  else
  {
    // Throw the error message
    throw coral::LFCReplicaService::LFCException(sstrerror(error), callname);
  }

  std::string& host = m_hosts[m_starthost];

  LOG_DEBUG("Try to connect to LFC host " << host);

  return true;
}

//-----------------------------------------------------------------------------
