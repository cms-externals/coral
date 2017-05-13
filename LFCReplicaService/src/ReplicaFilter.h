#ifndef LFCREPLICASERVICE_REPLICAFILTER_H
#define LFCREPLICASERVICE_REPLICAFILTER_H

#include "RelationalAccess/AccessMode.h"
#include "ReplicaStatus.h"
#include "ReplicaSet.h"

#include <string>

namespace coral {

  namespace LFCReplicaService {

    class ReplicaFilter {

    public:
      //default constructor
      ReplicaFilter();
      //special constructor
      ReplicaFilter(coral::AccessMode&, std::string&, ReplicaStatus);
      //copy constructor
      ReplicaFilter(const ReplicaFilter& filter);

      void setPhysicalConnectionString(const std::string& physicalConnectionString);

      void setAuthenticationMechanism(const std::string& authenticationMechanism);

      void setUserName(const std::string& userName);

      void setServerName(const std::string& serverName);

      void setStatus(ReplicaStatus status);

      void setAccessMode(coral::AccessMode accessMode);

      bool select(const ReplicaDescription& replica) const;

    private:

      std::pair<std::string,bool>  m_physicalConnectionStringFilter;

      std::pair<std::string,bool>  m_authenticationMechanismFilter;

      std::pair<std::string,bool>  m_userNameFilter;

      std::pair<std::string,bool>  m_passwordFilter;

      std::pair<std::string,bool>  m_serverNameFilter;

      std::pair<ReplicaStatus,bool>  m_statusFilter;

      std::pair<coral::AccessMode,bool>  m_accessModeFilter;

      bool m_doFilter;

    };

    inline
    void coral::LFCReplicaService::ReplicaFilter::setPhysicalConnectionString(const std::string& physicalConnectionString){
      m_physicalConnectionStringFilter.first = physicalConnectionString;
      m_physicalConnectionStringFilter.second = true;
      m_doFilter = true;
    }

    inline
    void coral::LFCReplicaService::ReplicaFilter::setAuthenticationMechanism(const std::string& authenticationMechanism){
      m_authenticationMechanismFilter.first = authenticationMechanism;
      m_authenticationMechanismFilter.second = true;
      m_doFilter = true;
    }

    inline
    void coral::LFCReplicaService::ReplicaFilter::setUserName(const std::string& userName){
      m_userNameFilter.first = userName;
      m_userNameFilter.second = true;
      m_doFilter = true;
    }

    inline
    void coral::LFCReplicaService::ReplicaFilter::setServerName(const std::string& serverName){
      m_serverNameFilter.first = serverName;
      m_serverNameFilter.second = true;
      m_doFilter = true;
    }

    inline
    void coral::LFCReplicaService::ReplicaFilter::setStatus(coral::LFCReplicaService::ReplicaStatus status){
      m_statusFilter.first = status;
      m_statusFilter.second = true;
      m_doFilter = true;
    }

    inline
    void coral::LFCReplicaService::ReplicaFilter::setAccessMode(coral::AccessMode accessMode){
      m_accessModeFilter.first = accessMode;
      m_accessModeFilter.second = true;
      m_doFilter = true;
    }

  }

}

#endif
