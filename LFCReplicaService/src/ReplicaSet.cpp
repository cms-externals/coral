#include "RelationalAccess/IAuthenticationCredentials.h"
#include "ReplicaSet.h"
#include "XMLLookupFileContent.h"
#include "XMLAuthenticationFileContent.h"
#include <iostream>
#include <iomanip>

#include "LFCReplica.h"
#include "LFCNode.h"
#include "LFCPhysicalConnectionString.h"
#include "LFCCredentialString.h"

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

ReplicaDescription::ReplicaDescription(const LFCReplica& replica, const std::string& guid)
  : m_guid( guid )
  , m_physicalConnectionString( "" )
  , m_authenticationMechanism( replica.poolname )
  , m_userName("")
  , m_password("")
  , m_serverName( replica.host )
  , m_status( replica.status ? On : Off )
  , m_accessMode( replica.f_type == 'R' ? coral::ReadOnly : coral::Update )
{
  if( replica.sfn.empty() )
    throw ReplicaDescriptionException("SFN retrieved from LFC database is empty.",
                                      "LFCDbReplicaMapping::dbReplica()");

  if( m_authenticationMechanism == "None" ) m_authenticationMechanism.clear();

  LFCPhysicalConnectionString lfcPfn( replica.sfn );

  if(!lfcPfn.isCoralPfn())
    throw ReplicaDescriptionException("PFN retrieved from LFC database is not a CORAL entry: \"" + replica.sfn + "\".", "ReplicaDescription::ReplicaDescription()");

  if(!lfcPfn.parse())
    throw ReplicaDescriptionException("PFN retrieved from LFC database could not be parsed successfully: \"" + replica.sfn + "\".", "ReplicaDescription::ReplicaDescription()");

  // Do the check only if guid is not empty
  if( !m_guid.empty() )
  {
    if(m_guid != lfcPfn.guid())
      throw ReplicaDescriptionException("Guid missmatch", "ReplicaDescription::ReplicaDescription()");
  }

  m_physicalConnectionString = lfcPfn.connectionString();
  m_userName = lfcPfn.userName();
  m_password = LFCCredentialString::password( replica.fs, lfcPfn.guid() );
}

//-----------------------------------------------------------------------------

ReplicaSet::~ReplicaSet()
{
  for(std::set<ReplicaDescription*>::iterator iR=m_descriptions.begin(); iR!=m_descriptions.end(); iR++) {
    delete *iR;
  }
}

//-----------------------------------------------------------------------------

void
ReplicaSet::appendReplica( const std::string& logicalName,
                           const std::string& role,
                           ReplicaDescription* replica )
{
  if(replica) {
    m_descriptions.insert(replica);
    std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >::iterator iLm=m_replicaMap.find(logicalName);
    if(iLm==m_replicaMap.end()) {
      std::map<std::string,std::multimap<std::string,ReplicaDescription*> > newMap;
      std::multimap<std::string,ReplicaDescription*> newReplicas;
      newReplicas.insert(std::make_pair(replica->physicalConnectionString(),replica));
      newMap.insert(std::make_pair(role,newReplicas));
      m_replicaMap.insert(std::make_pair(logicalName,newMap));
    } else {
      std::map<std::string,std::multimap<std::string,ReplicaDescription*> >::iterator iRm=iLm->second.find(role);
      if(iRm==iLm->second.end()) {
        std::multimap<std::string,ReplicaDescription*> reps;
        reps.insert(std::make_pair(replica->physicalConnectionString(),replica));
        iLm->second.insert(std::make_pair(role,reps));
      } else {
        iRm->second.insert(std::make_pair(replica->physicalConnectionString(),replica));
      }
    }
  }
}

//-----------------------------------------------------------------------------

void
ReplicaSet::prettyPrint(std::ostream& out) const
{
  std::string lcsTag("log.connection");
  std::string roleTag("role");
  std::string pcsTag("ph.connection");
  std::string serverTag("server");
  std::string statusTag("status");
  std::string authTag("authent.");
  std::string userTag("username");
  std::string pwdTag("password");

  size_t lcsMaxW =lcsTag.length(), roleMaxW = roleTag.length();
  size_t pcsMaxW=pcsTag.length(), svMaxW = serverTag.length();
  size_t usMaxW = userTag.length(), pwMaxW = pwdTag.length(), amMaxW = authTag.length();

  // first find the max lenghts
  for(std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >::const_iterator iRm=
        m_replicaMap.begin();
      iRm!=m_replicaMap.end();
      ++iRm) {
    if(iRm->first.length()>lcsMaxW) lcsMaxW = iRm->first.length();
    for(std::map<std::string,std::multimap<std::string,ReplicaDescription*> >::const_iterator iRole = iRm->second.begin();
        iRole!=iRm->second.end();
        ++iRole) {
      if(iRole->first.length() > roleMaxW) roleMaxW = iRole->first.length();
      for(std::multimap<std::string,ReplicaDescription*>::const_iterator iDbRep = iRole->second.begin();
          iDbRep!=iRole->second.end();
          ++iDbRep) {
        if((iDbRep->second)->physicalConnectionString().length()>pcsMaxW) pcsMaxW = (iDbRep->second)->physicalConnectionString().length();
        if((iDbRep->second)->serverName().length()>svMaxW) svMaxW = (iDbRep->second)->serverName().length();
        if((iDbRep->second)->userName().length()>usMaxW ) usMaxW= (iDbRep->second)->userName().length();
        if((iDbRep->second)->password().length()>pwMaxW ) pwMaxW = (iDbRep->second)->password().length();
        if((iDbRep->second)->authenticationMechanism().length()>amMaxW ) amMaxW = (iDbRep->second)->authenticationMechanism().length();
      }
    }
  }
  // then print formatted
  out << "Found "<<m_descriptions.size()<<" replica(s)."<<std::endl;
  std::string space("  ");
  if(m_descriptions.size()) {
    out << std::setiosflags(std::ios_base::left);
    out <<space<<std::setw(lcsMaxW)<<lcsTag;
    out <<space<<std::setw(roleMaxW)<<roleTag;
    out <<space<<std::setw(pcsMaxW)<<pcsTag;
    out <<space<<std::setw(4)<<"mode";
    out <<space<<std::setw(svMaxW)<<serverTag;
    out <<space<<std::setw(6)<<"status";
    out <<space<<std::setw(amMaxW)<<authTag;
    out <<space<<std::setw(usMaxW)<<userTag;
    //out <<space<<std::setw(pwMaxW)<<pwdTag;
    out <<std::endl;
  }
  for(std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >::const_iterator iRm=
        m_replicaMap.begin();
      iRm!=m_replicaMap.end();
      ++iRm) {
    out <<space<<std::setfill('-')<<std::setw(lcsMaxW)<<"";
    out <<space<<std::setw(roleMaxW)<<"";
    out <<space<<std::setw(pcsMaxW)<<"";
    out <<space<<std::setw(4)<<"";
    out <<space<<std::setw(svMaxW)<<"";
    out <<space<<std::setw(6)<<"";
    out <<space<<std::setw(amMaxW)<<"";
    out <<space<<std::setw(usMaxW)<<"";
    //out <<space<<std::setw(pwMaxW)<<"";
    out <<std::endl;
    out << std::setfill(' ');
    for(std::map<std::string,std::multimap<std::string,ReplicaDescription*> >::const_iterator iRole = iRm->second.begin();
        iRole!=iRm->second.end();
        ++iRole) {
      for(std::multimap<std::string,ReplicaDescription*>::const_iterator iDbRep = iRole->second.begin();
          iDbRep!=iRole->second.end();
          ++iDbRep) {
        std::string status("OFF");
        if((iDbRep->second)->status()==coral::LFCReplicaService::On) status = "ON";
        std::string authMech = (iDbRep->second)->authenticationMechanism();
        if(authMech.empty()) authMech = "none";
        std::string accessMode("R");
        if((iDbRep->second)->accessMode()==coral::Update) accessMode = "RW";
        out <<space<<std::setw(lcsMaxW)<<iRm->first;
        out <<space<<std::setw(roleMaxW)<<iRole->first;
        out <<space<<std::setw(pcsMaxW)<<(iDbRep->second)->physicalConnectionString();
        out <<space<<std::setw(4)<<accessMode;
        out <<space<<std::setw(svMaxW)<<(iDbRep->second)->serverName();
        out <<space<<std::setw(6)<<status;
        out <<space<<std::setw(amMaxW)<<authMech;
        out <<space<<std::setw(usMaxW)<<(iDbRep->second)->userName();
        //out <<space<<std::setw(pwMaxW)<<(iDbRep->second)->password();
        out << std::endl;
      }
    }
  }
}

void coral::LFCReplicaService::ReplicaSet::XMLPrint(std::ostream& lookupOut,
                                                    std::ostream& authenticationOut) const {
  coral::LFCReplicaService::XMLLookupFileContent lookupPrintOut(lookupOut);
  coral::LFCReplicaService::XMLAuthenticationFileContent authenticationPrintOut(authenticationOut);

  typedef std::map<std::string, std::map<std::string,ReplicaDescription*> > ReplicaList;
  ReplicaList replicas;
  typedef std::map<std::string,std::vector<std::pair<std::string,std::pair<std::string,std::string> > > > CredentialMap;
  CredentialMap credMap;

  for(std::map<std::string,std::map<std::string,std::multimap<std::string,ReplicaDescription*> > >::const_iterator iRm=
        m_replicaMap.begin();
      iRm!=m_replicaMap.end();
      ++iRm) {
    for(std::map<std::string,std::multimap<std::string,ReplicaDescription*> >::const_iterator iRole = iRm->second.begin();
        iRole!=iRm->second.end();
        ++iRole) {

      for(std::multimap<std::string,ReplicaDescription*>::const_iterator iDbRep = iRole->second.begin();
          iDbRep!=iRole->second.end();
          ++iDbRep) {
        ReplicaList::iterator iR = replicas.find(iRm->first);
        if(iR==replicas.end()) {
          std::map<std::string,ReplicaDescription*> mapEntry;
          mapEntry.insert(std::make_pair((iDbRep->second)->physicalConnectionString(),iDbRep->second));
          replicas.insert(std::make_pair(iRm->first,mapEntry));
        } else {
          iR->second.insert(std::make_pair((iDbRep->second)->physicalConnectionString(),iDbRep->second));
        }
        if(!(iDbRep->second)->authenticationMechanism().empty()) {
          CredentialMap::iterator iC = credMap.find((iDbRep->second)->physicalConnectionString());
          if(iC==credMap.end()) {
            std::vector<std::pair<std::string,std::pair<std::string,std::string> > > credVec;
            iC = credMap.insert(std::make_pair((iDbRep->second)->physicalConnectionString(),credVec)).first;
          }
          iC->second.push_back(std::make_pair(iRole->first,std::make_pair((iDbRep->second)->userName(),(iDbRep->second)->password())));
        }
      }
    }
  }

  for(ReplicaList::const_iterator iR = replicas.begin(); iR!=replicas.end(); ++iR) {
    lookupPrintOut.openLcsEntry(iR->first);
    for(std::map<std::string,ReplicaDescription*>::const_iterator iRD = iR->second.begin(); iRD!=iR->second.end(); ++iRD) {
      lookupPrintOut.addPcsEntry(iRD->first,iRD->second->authenticationMechanism(), iRD->second->accessMode());
    }
    lookupPrintOut.closeLcsEntry();
  }
  lookupPrintOut.close();
  lookupOut << std::flush;
  for(CredentialMap::const_iterator iC = credMap.begin(); iC!=credMap.end(); ++iC) {
    authenticationPrintOut.openConnectionEntry(iC->first);
    for(std::vector<std::pair<std::string,std::pair<std::string,std::string> > >::const_iterator iCP=iC->second.begin();
        iCP!=iC->second.end(); ++iCP) {
      if(iCP->first==coral::IAuthenticationCredentials::defaultRole()) {
        authenticationPrintOut.addCredentialEntry(iCP->second.first,iCP->second.second);
      } else {
        authenticationPrintOut.openRoleEntry(iCP->first);
        authenticationPrintOut.addCredentialEntry(iCP->second.first,iCP->second.second);
        authenticationPrintOut.closeRoleEntry();
      }
    }
    authenticationPrintOut.closeConnectionEntry();
  }
  authenticationPrintOut.close();
  authenticationOut << std::flush;
}
