#ifndef CORALCOMMON_IDEVCONNECTION_H
#define CORALCOMMON_IDEVCONNECTION_H

#include "RelationalAccess/IConnection.h"
#include "CoralBase/boost_thread_headers.h"
#include <set>

namespace coral {

  class IDevSession;

  class IDevConnection : public IConnection
  {
  public:
    /// Deregisters a session. This method is called at the destructor of IDevSession.
    void deregisterSession( IDevSession* session );

    /// Registers a new IDevSession object. This method is called by the constructor of IDevSession.
    void registerSession( IDevSession* session );

    /// The current size of the session pool
    size_t size() const;

  protected:
    /// Constructor. Initializes the session pool
    IDevConnection();

    /// Destructor. Invalidates all active sessions and cleans up the session pool
    virtual ~IDevConnection();

    /// Invalidates all active sessions
    void invalidateAllSessions();

  private:
    /// The sessions
    std::set< IDevSession* > m_sessions;

    /// The mutex protecting the session pool
    mutable boost::mutex m_mutex;
  };

}

#endif
