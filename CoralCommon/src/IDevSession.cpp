#include "CoralCommon/IDevSession.h"
#include "CoralCommon/IDevConnection.h"

coral::IDevSession::IDevSession( coral::IDevConnection& connection ) :
  m_connection( connection ),
  m_valid( true ),
  m_mutex()
{
  m_connection.registerSession( this );
}

coral::IDevSession::~IDevSession()
{
  if ( this->isSessionValid() )
    m_connection.deregisterSession( this );
}


void
coral::IDevSession::invalidateSession()
{
  boost::mutex::scoped_lock lock( m_mutex );
  m_valid = false;
}


bool
coral::IDevSession::isSessionValid() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_valid;
}

std::string
coral::IDevSession::remoteServerVersion() const
{
  boost::mutex::scoped_lock lock( m_mutex );
  return m_connection.serverVersion();
}
