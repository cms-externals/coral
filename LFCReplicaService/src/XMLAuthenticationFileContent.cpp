#include "RelationalAccess/IAuthenticationCredentials.h"
#include "XMLAuthenticationFileContent.h"
#include <iomanip>

coral::LFCReplicaService::XMLAuthenticationFileContent::XMLAuthenticationFileContent(std::ostream& out) :
  m_out(out),
  m_connectionListOpen(false),
  m_connectionEntryOpen(false),
  m_roleEntryOpen(false),
  m_ind(0){
  m_out << "<?xml version=\"1.0\" ?>"<<std::endl;
  m_out << "<connectionlist>"<<std::endl;
  m_connectionListOpen = true;
}

bool coral::LFCReplicaService::XMLAuthenticationFileContent::openConnectionEntry(const std::string& pcs){
  bool ret = false;
  if(m_connectionListOpen && !m_connectionEntryOpen) {
    m_out << std::endl;
    m_ind+=2;
    m_out << std::setw(m_ind)<<"";
    m_out << "<connection name=\""<<pcs<<"\" >"<<std::endl;
    m_connectionEntryOpen=true;
    ret = true;
  }
  return ret;
}

bool coral::LFCReplicaService::XMLAuthenticationFileContent::closeConnectionEntry(){
  bool ret = false;
  if(m_connectionEntryOpen) {
    m_out << std::setw(m_ind)<<"";
    m_out << "</connection>"<<std::endl;
    m_ind-=2;
    ret = true;
    m_connectionEntryOpen = false;
  }
  return ret;
}

bool coral::LFCReplicaService::XMLAuthenticationFileContent::openRoleEntry(const std::string& roleName){
  bool ret = false;
  if(m_connectionEntryOpen && !m_roleEntryOpen) {
    m_ind+=2;
    m_out << std::setw(m_ind)<<"";
    m_out << "<role name=\""<<roleName<<"\" >"<<std::endl;
    m_roleEntryOpen=true;
    ret = true;
  }
  return ret;

}

bool coral::LFCReplicaService::XMLAuthenticationFileContent::closeRoleEntry(){
  bool ret = false;
  if(m_roleEntryOpen) {
    m_out << std::setw(m_ind)<<"";
    m_out << "</role>"<<std::endl;
    m_ind-=2;
    ret = true;
    m_roleEntryOpen = false;
  }
  return ret;
}

bool coral::LFCReplicaService::XMLAuthenticationFileContent::addCredentialEntry(const std::string& userName,
                                                                                const std::string& password ){
  bool ret = false;
  if(m_connectionEntryOpen) {
    m_out << std::setw(m_ind+2)<<"";
    m_out << "<parameter name=\""<<coral::IAuthenticationCredentials::userItem()<<"\" value=\""<<userName<<"\" />"<<std::endl;
    m_out << std::setw(m_ind+2)<<"";
    m_out << "<parameter name=\""<<coral::IAuthenticationCredentials::passwordItem()<<"\" value=\""<<password<<"\" />"<<std::endl;
    ret = true;
  }
  return ret;
}

void coral::LFCReplicaService::XMLAuthenticationFileContent::close(){
  if(m_connectionListOpen) {
    if(m_connectionEntryOpen) {
      if(m_roleEntryOpen) {
        closeRoleEntry();
      }
      closeConnectionEntry();
    }
    m_out << std::endl;
    m_out << "</connectionlist>"<<std::endl;
  }
  m_connectionListOpen = false;
}
