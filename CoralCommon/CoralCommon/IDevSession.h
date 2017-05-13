#ifndef CORALCOMMON_IDEVSESSION_H
#define CORALCOMMON_IDEVSESSION_H

#include "RelationalAccess/ISession.h"
#include "CoralBase/boost_thread_headers.h"

namespace coral {

  class IDevConnection;

  class IDevSession : public ISession
  {
  public:
    /// Invalidates the session. Called by IDevConnection::invalidateAllSessions.
    void invalidateSession();

  protected:
    /// Constructor.
    explicit IDevSession( IDevConnection& connection );

    /// Destructor.
    virtual ~IDevSession();

    /// Checks the validity of the session.
    bool isSessionValid() const;

    /**
     * Returns the server version for the remote session.
     * For plugins establishing a database connection through a middle-tier
     * (e.g. CoralAccess), this method is overloaded because the value
     * is discovered only when establishing the remote session.
     */
    virtual std::string remoteServerVersion() const;

  private:
    /// The reference to the parent connection
    IDevConnection& m_connection;

    /// The validity flag
    bool m_valid;

    /// The mutex protecting the validity flag
    mutable boost::mutex m_mutex;
  };

}

#endif
