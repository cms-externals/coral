#include "uuid/uuid.h" // Keep it at the top (bug #35692 on Windows)
#include <cstdlib>
#include <map>
#include <sstream>

#include "CoralBase/MessageStream.h"
#include "CoralBase/boost_thread_headers.h"
#include "RelationalAccess/AuthenticationServiceException.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/SessionException.h"
#include "SessionHandle.h"

#ifdef _WIN32
#pragma warning ( disable : 4355 ) // 'this' used in base member initializer list
#endif

/// constructor
coral::ConnectionService::SessionHandle::SessionHandle( const std::string& connectionServiceName ) :
  m_connectionServiceName( connectionServiceName ),
  m_session(),
  m_info( new SessionSharedInfo( connectionServiceName ) ),
  m_connection( connectionServiceName ),
  m_transactionProxy( *this )
{
  coral_uuidt guid;
  ::uuid_generate_time( guid );
  char strGuid[37];
  ::uuid_unparse( guid,strGuid );
  m_info->m_sessionId = std::string(strGuid);
}

/// constructor
coral::ConnectionService::SessionHandle::SessionHandle( const std::string& connectionServiceName,
                                                        const ConnectionHandle& connection ) :
  m_connectionServiceName( connectionServiceName ),
  m_session(),
  m_info( new SessionSharedInfo( connectionServiceName ) ),
  m_connection(connection),
  m_transactionProxy( *this )
{
  coral_uuidt guid;
  ::uuid_generate_time( guid );
  char strGuid[37];
  ::uuid_unparse( guid,strGuid );
  m_info->m_sessionId = std::string(strGuid);
}

/// destructor
coral::ConnectionService::SessionHandle::~SessionHandle()
{
}

coral::ConnectionService::SessionHandle::SessionHandle( const SessionHandle& rhs ) :
  m_connectionServiceName( rhs.m_connectionServiceName ),
  m_session(rhs.m_session),
  m_info(rhs.m_info),
  m_connection(rhs.m_connection),
  m_transactionProxy( *this )
{
}

coral::ConnectionService::SessionHandle&
coral::ConnectionService::SessionHandle::operator=( const SessionHandle& rhs )
{
  m_connectionServiceName = rhs.m_connectionServiceName;
  m_session = rhs.m_session;
  m_info = rhs.m_info;
  m_connection = rhs.m_connection;
  return *this;
}

/// returns
coral::ConnectionService::SessionHandle::operator bool() const
{
  return m_session.get()!=0;
}

/// initialize the session
bool
coral::ConnectionService::SessionHandle::open( const std::string& schemaName,
                                               const std::string& userName,
                                               const std::string& password,
                                               coral::AccessMode accessMode )
{
  coral::MessageStream log( m_connectionServiceName );
  //log << coral::Info << "SessionHandle for sessionID=" << m_info->showSessionId() << " on connectionID=" << m_connection.showConnectionId() << " will be opened" << coral::MessageStream::endmsg;
  coral::ISession* sess = m_connection.newSession(schemaName,userName,password,accessMode,m_info->showSessionId());
  if(sess) {
    m_session = boost::shared_ptr<ISession>(sess);
    m_info->m_open = true;
    //log << coral::Info << "SessionHandle for sessionID=" << m_info->showSessionId() << " on connectionID=" << m_connection.showConnectionId() << " has been opened" << coral::MessageStream::endmsg;
  }
  return m_info->m_open;
}

void
coral::ConnectionService::SessionHandle::close()
{
  coral::MessageStream log( m_connectionServiceName );
  if(m_session) {
    if(m_info->m_open) {
      log << coral::Info << "User session with sessionID=" << m_info->showSessionId() << " will be ended on connectionID="<< m_connection.showConnectionId() << coral::MessageStream::endmsg; // Matches printout for startUserSession in ConnectionHandle.cpp
      //log << coral::Info << "SessionHandle for sessionID=" << m_info->showSessionId() << " on connectionID=" << m_connection.showConnectionId() << " will be closed" << coral::MessageStream::endmsg;
      m_session->endUserSession();
      //log << coral::Info << "SessionHandle for sessionID=" << m_info->showSessionId() << " on connectionID=" << m_connection.showConnectionId() << " has been closed" << coral::MessageStream::endmsg;
      m_info->m_open = false;
      m_session.reset();
    }
  }
  else
  {
    log << coral::Info << "SessionHandle for SessionID=" << m_info->showSessionId() << " was not open: no need to close it" << coral::MessageStream::endmsg;
  }
}

bool
coral::ConnectionService::SessionHandle::isOpen() const
{
  bool open = false;
  if(m_session && m_info->m_open) open = true;
  return open;
}

bool
coral::ConnectionService::SessionHandle::isValid() const
{
  bool valid = false;
  if(isOpen() && m_connection.isValid()) valid = true;
  return valid;
}

coral::ISession*
coral::ConnectionService::SessionHandle::physicalSession() const
{
  return m_session.get();
}

coral::ConnectionService::ConnectionHandle&
coral::ConnectionService::SessionHandle::connection()
{
  return m_connection;
}

const coral::ConnectionService::ConnectionHandle&
coral::ConnectionService::SessionHandle::connection() const
{
  return m_connection;
}

const std::string
coral::ConnectionService::SessionHandle::SessionSharedInfo::showSessionId( bool last ) const
{
  static char* simpleIDs = ::getenv( "CORAL_CONNECTIONSVC_SHOWSIMPLEIDS" );
  if ( simpleIDs && m_sessionId != "" )
  {
    static boost::mutex mutexForMap;
    boost::mutex::scoped_lock lock( mutexForMap );
    static std::map<Guid,std::string> idMap;
    static unsigned long lastId = 0;
    std::map<Guid,std::string>::iterator idIt = idMap.find( m_sessionId );
    if ( idIt != idMap.end() )
    {
      // GUID is already mapped
      std::string showId = idIt->second;
      if ( last ) idMap.erase( m_sessionId );
      return showId;
    }
    else if ( !last )
    {
      // GUID is not mapped and will be reused: map it
      std::stringstream msg;
      msg << "'Session#" << ++lastId << "'";
      idMap[m_sessionId] = msg.str();
      coral::MessageStream log( m_connectionServiceName );
      log << coral::Debug << "Session GUID=" << m_sessionId << " mapped to sessionID=" << msg.str() << coral::MessageStream::endmsg;
      return msg.str();
    }
    else
    {
      // GUID is not mapped and will not be reused: do not map it
      return m_sessionId;
    }
  }
  return m_sessionId;
}
