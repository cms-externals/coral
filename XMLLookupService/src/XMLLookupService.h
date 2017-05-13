#ifndef XMLAUTHENTITACTIONSERVICE_XMLAUTHENTITACTIONSERVICE_H
#define XMLAUTHENTITACTIONSERVICE_XMLAUTHENTITACTIONSERVICE_H

#include "RelationalAccess/ILookupService.h"

#include "CoralKernel/Service.h"
#include "CoralKernel/Property.h"

#include <map>
#include <vector>
#include "CoralBase/boost_thread_headers.h"

namespace coral
{

  namespace XMLLookupService
  {

    /**
     * @class XMLLookupService XMLLookupService.h
     *
     * A simple implementation of the ILookupService interface based on reading an XML file
     */
    class XMLLookupService : public coral::Service,
                             virtual public coral::ILookupService
    {
    public:
      /// Standard Constructor
      XMLLookupService( const std::string& componentName );

      /// Standard Destructor
      virtual ~XMLLookupService();

    public:
      /**
       * Performs a lookup for a given logical connection string.
       */
      coral::IDatabaseServiceSet* lookup( const std::string& logicalName,
                                          coral::AccessMode accessMode = coral::Update,
                                          std::string authenticationMechanism = "" ) const;

    public:
      /// Sets the input file name
      void setInputFileName(  const std::string& inputFileName );

    private:
      /// Service framework related initialization
      bool initialize();

      /// Verifies the existence of the authentication file
      std::string verifyFileName() const;

    private:
      /// Flag indicating whether the service has been initialized
      bool m_isInitialized;

      /// The input file with the data
      std::string m_inputFileName;

      /// the mutex lock
      mutable boost::mutex m_mutexLock;

      coral::Property::CallbackID m_callbackID;

      /// The data
      class DbEntry {
      public:
        DbEntry( std::string _connection = "",
                 std::string _authenticationMechanism = "",
                 coral::AccessMode _accessMode = coral::Update ) :
          connection( _connection ),
          authenticationMechanism( _authenticationMechanism ),
          accessMode( _accessMode ),
          serviceParameters()
        {}
        DbEntry( const DbEntry& rhs ) :
          connection( rhs.connection ),
          authenticationMechanism( rhs.authenticationMechanism ),
          accessMode( rhs.accessMode ),
          serviceParameters( rhs.serviceParameters )
        {}
        DbEntry& operator=( const DbEntry& rhs ){
          connection = rhs.connection;
          authenticationMechanism = rhs.authenticationMechanism;
          accessMode = rhs.accessMode;
          serviceParameters = rhs.serviceParameters;
          return *this;
        }
        std::string connection;
        std::string authenticationMechanism;
        coral::AccessMode accessMode;
        std::map<std::string,std::string> serviceParameters;
      };
      std::map< std::string, std::vector< DbEntry > > m_data;
    };

  }

}

#endif
