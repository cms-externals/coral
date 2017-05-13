#include "CoralCommon/IDevConnection.h"
#include "CoralCommon/IDevSession.h"

coral::IDevConnection::IDevConnection() :
  m_sessions(),
  m_mutex()
{
}


coral::IDevConnection::~IDevConnection()
{
  this->invalidateAllSessions();
}


void
coral::IDevConnection::deregisterSession( coral::IDevSession* session )
{
  boost::mutex::scoped_lock lock( m_mutex );
  m_sessions.erase( session );
}


void
coral::IDevConnection::registerSession( coral::IDevSession* session )
{
  boost::mutex::scoped_lock lock( m_mutex );
  m_sessions.insert( session );
}

size_t
coral::IDevConnection::size() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_sessions.size();
}


void
coral::IDevConnection::invalidateAllSessions()
{
  boost::mutex::scoped_lock lock( m_mutex );

  for ( std::set< coral::IDevSession* >::iterator iSession = m_sessions.begin();
        iSession != m_sessions.end(); ++iSession ) {
    (*iSession)->invalidateSession();
  }
  m_sessions.clear();
}
