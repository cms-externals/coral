#ifndef LFCREPLICASERVICE_LFCSESSION_H
#define LFCREPLICASERVICE_LFCSESSION_H 1

#include <string>
#include <vector>

namespace coral {

  namespace LFCReplicaService {

    /**
     *  @class LFCConnection
     *  @author Alexander Kalkhof
     *
     *  This class handles all LFC connections
     *  The LFC hosts can be setup using the env variables
     *
     *  Either to set one specific host LFC_HOST=myhost.net
     *
     *  or to set several hosts CORAL_LFC_HOSTS="myhost1.net;myhost2.net"
     *
     *  The class tries to connect in the order the hosts are set
     *  Additional retry period and timeout can be configured for each host
     *
     *  CORAL_LFC_RETRY_PERIOD=n , CORAL_LFC_RETRY_TIMEOUT=n , all in seconds
     *
     */
    class LFCConnection
    {

    public:

      LFCConnection();

      ~LFCConnection();

      bool retry(const std::string& callname, bool status);

    private:

      void setCurrentHost();

    private:

      std::vector< std::string > m_hosts;

      size_t m_retry_period;

      size_t m_timeout;

      time_t m_startime;

      size_t m_starthost;

    };

    // Forward declaration
    class LFCReplicaSet;
    class LFCReplica;

    /**
     *  @class LFCSession
     *  @author Alexander Kalkhof
     *
     *  This class handles all LFC sessions
     *  The LFC hosts can be setup using the env variables
     *
     *  Either to set one specific host LFC_HOST=myhost.net
     *
     *  or to set several hosts CORAL_LFC_HOSTS="myhost1.net;myhost2.net"
     *
     *  The class tries to connect in the order the hosts are set
     *  Additional retry period and timeout can be configured for each host
     *
     *  CORAL_LFC_RETRY_PERIOD=n , CORAL_LFC_RETRY_TIMEOUT=n , all in seconds
     *
     */
    class LFCSession
    {

    public:

      /**
       * Default constructor
       */
      LFCSession();

      /**
       * Default destructor
       */
      ~LFCSession();

      /**
       * Starts a LFC session
       *
       * force defines that the session will be forced to start
       * use force=true if you have lfc calls following
       * use force=false if you want to group methods into one session
       * method waits until a startSession with force=true was called
       * to initialize the real LFC session
       *
       * the mechanism tries to avoid not needed sessions to the LFC
       * group as much as possible
       */
      void startSession(bool force);

      /**
       * Ends a LFC session
       *
       * waits until the level of the first call of startSession
       * to stop the real LFC session
       */
      void endSession();

      /**
       * Get all replicas without starting a session
       *
       */
      static void getReplicas( LFCReplicaSet&,
                               const std::string& poolname,
                               const std::string& host,
                               const std::string& fs );

      static void deleteReplica( LFCReplica& );

      static void addReplica( LFCReplica& );

      static void setReplicaStatus( LFCReplica&, bool status );

    private:

      bool tryConnect(const char* host);

    private:

      size_t m_openctn;

      bool m_started;

      std::vector< std::string > m_hosts;

      size_t m_retry_period;

      size_t m_timeout;

    };

  }

}

#endif
