#ifndef LFCREPLICASERVICE_REPLICASET_H
#define LFCREPLICASERVICE_REPLICASET_H

#include "CoralBase/Exception.h"
#include "RelationalAccess/AccessMode.h"
#include "ReplicaStatus.h"
#include <map>
#include <set>
#include <vector>
#include <string>

namespace coral {

  namespace LFCReplicaService {

    class LFCReplica;

    class LFCReplicaInfo;

    class LFCNode;

    class ReplicaDescription {

    public:

      ReplicaDescription();

      ReplicaDescription(const std::string& physicalConnectionString,
                         const std::string& authenticationMechanism,
                         const std::string& userName,
                         const std::string& password,
                         const std::string& serverName,
                         ReplicaStatus status,
                         coral::AccessMode accessMode);

      ReplicaDescription(const std::string& guid,
                         const std::string& physicalConnectionString,
                         const std::string& authenticationMechanism,
                         const std::string& userName,
                         const std::string& password,
                         const std::string& serverName,
                         ReplicaStatus status,
                         coral::AccessMode accessMode);

      ReplicaDescription(const LFCReplica&, const std::string& guid);

      const std::string& guid() const { return m_guid; };

      const std::string& physicalConnectionString() const { return m_physicalConnectionString; };

      const std::string& authenticationMechanism() const { return m_authenticationMechanism; };

      const std::string& userName() const { return m_userName; };

      const std::string& password() const { return m_password; };

      const std::string& serverName() const { return m_serverName; };

      ReplicaStatus status() const { return m_status; };

      coral::AccessMode accessMode() const { return m_accessMode; };

    private:

      std::string m_guid;

      std::string m_physicalConnectionString;

      std::string m_authenticationMechanism;

      std::string m_userName;

      std::string m_password;

      std::string m_serverName;

      ReplicaStatus m_status;

      coral::AccessMode m_accessMode;

    };

    class ReplicaDescriptionException : public coral::Exception {
    public:

      /// Constructors
      ReplicaDescriptionException(const std::string& message, const std::string& methodName)
        : Exception( message, methodName, "CORAL/Services/LFCReplicaService" )
      {
      }

      /// Destructor
      virtual ~ReplicaDescriptionException() throw()
      {
      }

    };


    class ReplicaSet {

    public:

      ReplicaSet();

      virtual ~ReplicaSet();

      const std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >& replicaMap() const;

      void appendReplica(const std::string& logicalName,
                         const std::string& role,
                         ReplicaDescription* replica );

      void prettyPrint(std::ostream& out) const;

      void XMLPrint(std::ostream& lookupOut, std::ostream& authenticationOut) const;

    private:

      std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > > m_replicaMap;

      std::set<ReplicaDescription*> m_descriptions;
    };

  }
}

inline
coral::LFCReplicaService::ReplicaSet::ReplicaSet() : m_replicaMap(),m_descriptions(){
}

inline
const std::map<std::string,std::map<std::string,std::multimap<std::string,coral::LFCReplicaService::ReplicaDescription*> > >& coral::LFCReplicaService::ReplicaSet::replicaMap() const {
  return m_replicaMap;
}

inline
coral::LFCReplicaService::ReplicaDescription::ReplicaDescription() :
  m_guid(""),
  m_physicalConnectionString(""),
  m_authenticationMechanism(""),
  m_userName(""),
  m_password(""),
  m_serverName(""),
  m_status(coral::LFCReplicaService::On),
  m_accessMode(coral::Update){
}

inline
coral::LFCReplicaService::ReplicaDescription::ReplicaDescription(const std::string& physicalConnectionString,
                                                                 const std::string& authenticationMechansim,
                                                                 const std::string& userName,
                                                                 const std::string& password,
                                                                 const std::string& serverName,
                                                                 coral::LFCReplicaService::ReplicaStatus status,
                                                                 coral::AccessMode accessMode ) :
  m_guid(""),
  m_physicalConnectionString(physicalConnectionString),
  m_authenticationMechanism(authenticationMechansim),
  m_userName(userName),
  m_password(password),
  m_serverName(serverName),
  m_status(status),
  m_accessMode(accessMode){
}

inline
coral::LFCReplicaService::ReplicaDescription::ReplicaDescription(const std::string& guid,
                                                                 const std::string& physicalConnectionString,
                                                                 const std::string& authenticationMechansim,
                                                                 const std::string& userName,
                                                                 const std::string& password,
                                                                 const std::string& serverName,
                                                                 coral::LFCReplicaService::ReplicaStatus status,
                                                                 coral::AccessMode accessMode ) :
  m_guid(guid),
  m_physicalConnectionString(physicalConnectionString),
  m_authenticationMechanism(authenticationMechansim),
  m_userName(userName),
  m_password(password),
  m_serverName(serverName),
  m_status(status),
  m_accessMode(accessMode){
}

#endif
