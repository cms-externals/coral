#include "LFCPhysicalConnectionString.h"
#include "Guid.h"
#include <sstream>

const std::string coral::LFCReplicaService::LFCPhysicalConnectionString::CoralPfnTag = "/coral_db/";

coral::LFCReplicaService::LFCPhysicalConnectionString::LFCPhysicalConnectionString() : m_pfn(""),
                                                                                       m_dbPcs(""),
                                                                                       m_userName(""),
                                                                                       m_guid(""){
}

coral::LFCReplicaService::LFCPhysicalConnectionString::LFCPhysicalConnectionString(const std::string& pfn) : m_pfn(pfn),
                                                                                                             m_dbPcs(""),
                                                                                                             m_userName(""),
                                                                                                             m_guid(""){
}

bool
coral::LFCReplicaService::LFCPhysicalConnectionString::isCoralPfn(){
  if(m_pfn.empty()) return false;
  return m_pfn.substr(0, CoralPfnTag.size())==CoralPfnTag;
}

bool
coral::LFCReplicaService::LFCPhysicalConnectionString::parse(){
  bool parsed = false;
  // format for the lfc pfn is= "/[CoralPfnTag]/[Guid]/[UserName]/[PhysicalConnectionString]"
  if(isCoralPfn()) {
    std::string::size_type cursor = CoralPfnTag.size();
    std::string::size_type endGuidPos = m_pfn.find("/",cursor);
    if(endGuidPos!=std::string::npos) {
      m_guid = m_pfn.substr( cursor, endGuidPos - cursor );
      cursor = endGuidPos+1;
      std::string::size_type endUserPos = m_pfn.find("/",cursor);
      if(endUserPos!=std::string::npos) {
        m_userName = m_pfn.substr( cursor, endUserPos - cursor );
        cursor = endUserPos+1;
        m_dbPcs = m_pfn.substr(cursor);
        parsed = true;
      }
    }
    if(parsed) {
      if(!Guid::isGuid(m_guid)) parsed = false;
      if(m_dbPcs.empty()) parsed = false;
    }
  }
  return parsed;
}

const std::string& coral::LFCReplicaService::LFCPhysicalConnectionString::connectionString() const {
  return m_dbPcs;
}

const std::string& coral::LFCReplicaService::LFCPhysicalConnectionString::userName() const {
  return m_userName;
}

const std::string& coral::LFCReplicaService::LFCPhysicalConnectionString::guid() const {
  return m_guid;
}

std::string coral::LFCReplicaService::LFCPhysicalConnectionString::pfn(const std::string& physicalConnectionString,
                                                                       const std::string& userName,
                                                                       const std::string& guid){
  std::ostringstream pfnOs;
  pfnOs << CoralPfnTag;
  pfnOs << guid;
  pfnOs << "/";
  pfnOs << userName;
  pfnOs << "/";
  pfnOs << physicalConnectionString;
  return pfnOs.str();
}

std::string coral::LFCReplicaService::LFCPhysicalConnectionString::pfn(const std::string& physicalConnectionString,
                                                                       const std::string& userName){
  Guid newGuid;
  Guid::create(newGuid);
  return pfn(physicalConnectionString,userName,newGuid.toString());
}
