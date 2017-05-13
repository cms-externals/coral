#ifndef XMLAUTHENTITACTIONSERVICE_XMLAUTHENTITACTIONSERVICE_H
#define XMLAUTHENTITACTIONSERVICE_XMLAUTHENTITACTIONSERVICE_H

#include "RelationalAccess/IAuthenticationService.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Property.h"

#include <map>
#include <vector>
#include "CoralBase/boost_thread_headers.h"

namespace coral
{

  namespace XMLAuthenticationService {

    class DataSourceEntry;

    /**
     * @class AuthenticationService AuthenticationService.h
     *
     * A simple implementation of the IAuthenticationService interface based on reading an XMl file
     */
    class XMLAuthenticationService : public coral::Service,
                                     virtual public coral::IAuthenticationService
    {
    public:
      /// Standard Constructor
      XMLAuthenticationService( const std::string& componentName );

      /// Standard Destructor
      virtual ~XMLAuthenticationService();

    public:
      /**
       * Returns a reference to the credentials object for a given connection string.
       * If the connection string is not known to the service an UnknownConnectionException is thrown.
       */
      const coral::IAuthenticationCredentials& credentials( const std::string& connectionString ) const;

      /**
       * Returns a reference to the credentials object for a given connection string.
       * If the connection string is not known to the service an UnknownConnectionException is thrown.
       * If the role is not known to the service an UnknownRoleException is thrown.
       */
      const coral::IAuthenticationCredentials& credentials( const std::string& connectionString, const std::string& role ) const;

    public:
      /// Sets the input file name
      void setInputFileName(  const std::string& inputFileName );

    private:
      /// Service framework related initialization
      bool initialize();

      /// Parses an xml file
      bool processFile( const std::string& inputFileName );

      /// Verifies the existence of the authentication files
      std::vector< std::string > verifyFileName() const;

      /// Flag indicating whether the service has been initialized
      bool m_isInitialized;

      /// The input file with the data
      std::string m_inputFileName;

      /// The structure with the authentication data
      std::map< std::string, DataSourceEntry* > m_data;

      /// the mutex lock
      mutable boost::mutex m_mutexLock;

      coral::Property::CallbackID m_callbackID;
    };

  }

}

#endif
