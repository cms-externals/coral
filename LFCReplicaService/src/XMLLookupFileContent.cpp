#include "XMLLookupFileContent.h"
#include <iomanip>

coral::LFCReplicaService::XMLLookupFileContent::XMLLookupFileContent(std::ostream& out) : m_out(out),
                                                                                          m_serviceEntryOpen(false),
                                                                                          m_lcsEntryOpen(false),
                                                                                          m_ind(0){
  m_out << "<?xml version=\"1.0\" ?>"<<std::endl;
  m_out << "<servicelist>"<<std::endl;
  m_serviceEntryOpen = true;
}

bool coral::LFCReplicaService::XMLLookupFileContent::openLcsEntry(const std::string& lcs){
  bool ret = false;
  if(m_serviceEntryOpen && !m_lcsEntryOpen) {
    m_out << std::endl;
    m_ind+=2;
    m_out << std::setw(m_ind)<<"";
    m_out << "<logicalservice name=\""<<lcs<<"\" >"<<std::endl;
    m_lcsEntryOpen=true;
    ret = true;
  }
  return ret;
}

bool coral::LFCReplicaService::XMLLookupFileContent::closeLcsEntry(){
  bool ret = false;
  if(m_lcsEntryOpen) {
    m_out << std::setw(m_ind)<<"";
    m_out << "</logicalservice>"<<std::endl;
    m_ind-=2;
    ret = true;
    m_lcsEntryOpen = false;
  }
  return ret;
}

bool coral::LFCReplicaService::XMLLookupFileContent::addPcsEntry(const std::string& pcs,
                                                                 const std::string& authent,
                                                                 coral::AccessMode mode){
  bool ret = false;
  if(m_lcsEntryOpen) {
    m_out << std::setw(m_ind+2)<<"";
    m_out << "<service name=\""<<pcs<<"\" accessMode=\"";
    if(mode==coral::ReadOnly) {
      m_out<<"readonly";
    } else{
      m_out <<"update";
    }
    m_out <<"\" authentication=\""<<authent<<"\" />"<<std::endl;
    ret = true;
  }
  return ret;
}

void coral::LFCReplicaService::XMLLookupFileContent::close(){
  if(m_serviceEntryOpen) {
    if(m_lcsEntryOpen) {
      closeLcsEntry();
    }
    m_out << std::endl;
    m_out << "</servicelist>"<<std::endl;
  }
  m_serviceEntryOpen = false;
}
