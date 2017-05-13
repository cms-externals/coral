// -*-C++-*-
#ifndef CONNECTIONSERVICE_DATASOURCE_H
#define CONNECTIONSERVICE_DATASOURCE_H

#include "ConnectionParams.h"

#include "CoralBase/boost_thread_headers.h"

#include <map>
#include <vector>

namespace coral {

  class ICertificateData;

  namespace ConnectionService {

    class ConnectionServiceConfiguration;

    /// class providing data source paramters
    class DataSource {

    public:

      /// constructor
      DataSource( const ConnectionServiceConfiguration& serviceConfiguration );

      /// destructor
      virtual ~DataSource(){}

      /// loads the required plugin library and retrieves the connection parametes
      bool setupForConnection( ConnectionParams& connectionParameters, const std::string& role,
                               const coral::ICertificateData *cert );

    private:

      std::pair<std::string,std::string>* getCredentials( const std::string& connectionString,
                                                          const std::string& role,
                                                          const coral::ICertificateData *cert) const;

    private:

      /// the reference to the service configuration
      const ConnectionServiceConfiguration& m_serviceConfiguration;

    };
  }
}

#endif
