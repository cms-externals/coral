#include "XMLLookupFileParser.h"
#include "XMLAuthenticationFileParser.h"
#include "XMLReplicaImporter.h"
#include "ReplicaSet.h"
#include "AuthenticationCredentialSet.h"
#include "RelationalAccess/AuthenticationCredentials.h"
#include "CoralCommon/DatabaseServiceDescription.h"
#include "CoralCommon/URIParser.h"
#include "CoralCommon/URIException.h"
#include "CoralBase/MessageStream.h"

namespace coral {
  namespace LFCReplicaService {

    void cleanUpData(std::map<std::string,coral::AuthenticationCredentialSet*> authData,
                     std::map< std::string, std::vector< DatabaseServiceDescription* > > lookupData){
      for(std::map<std::string,coral::AuthenticationCredentialSet*>::iterator iA=authData.begin();
          iA != authData.end(); ++iA) {
        delete iA->second;
      }
      for(std::map< std::string, std::vector< DatabaseServiceDescription* > >::iterator iL=lookupData.begin();
          iL != lookupData.end(); ++iL) {
        for(std::vector< DatabaseServiceDescription* >::iterator iD=iL->second.begin();
            iD!=iL->second.end(); ++iD) {
          delete *iD;
        }
      }
    }

  }
}

coral::LFCReplicaService::ReplicaSet* coral::LFCReplicaService::XMLReplicaImporter::getData(){
  ReplicaSet* replicaSet = new ReplicaSet;
  coral::XMLAuthenticationFileParser authParser(m_serviceCallerName);
  std::map<std::string,coral::AuthenticationCredentialSet*> authData;
  authParser.parse(m_authenticationFile,authData);

  coral::XMLLookupFileParser lookupParser(m_serviceCallerName);
  std::map< std::string, std::vector< DatabaseServiceDescription* > > lookupData;
  lookupParser.parse(m_lookupFile,lookupData);

  for(std::map< std::string, std::vector< DatabaseServiceDescription* > >::const_iterator iLookupSet=lookupData.begin();
      iLookupSet!=lookupData.end(); ++iLookupSet) {
    std::string lcs = iLookupSet->first;
    for(std::vector< DatabaseServiceDescription* >::const_iterator iDbEntry=iLookupSet->second.begin();
        iDbEntry!=iLookupSet->second.end(); ++iDbEntry) {
      std::string pcs = (*iDbEntry)->connectionString();
      std::string host("");
      try{
        coral::URIParser parser;
        parser.setURI(pcs);
        host = parser.hostName();
        if(host.empty()) host = parser.databaseOrSchemaName();
      } catch (const coral::URIException& e) {
        delete replicaSet;
        cleanUpData(authData,lookupData);
        coral::MessageStream log( m_serviceCallerName );
        log << coral::Error <<"Failure processing connection="<<pcs<<": could not derive host name."<<coral::MessageStream::endmsg;
        throw coral::Exception("Could not derive host name from connection string: "+std::string(e.what()),
                               "XMLReplicaImporter::getData","LFCReplicaService");
      }
      std::string authenticationMethod = (*iDbEntry)->authenticationMechanism();
      coral::AccessMode accessMode = (*iDbEntry)->accessMode();
      if(authenticationMethod=="password") {
        std::map<std::string,coral::AuthenticationCredentialSet*>::const_iterator iAuthSet = authData.find(pcs);
        if(iAuthSet != authData.end()) {
          for(std::map< std::string, coral::AuthenticationCredentials* >::const_iterator iRoleEntry = iAuthSet->second->roleMap().begin();
              iRoleEntry!=iAuthSet->second->roleMap().end();
              ++iRoleEntry) {
            std::string role = iRoleEntry->first;
            std::string userName = iRoleEntry->second->valueForItem(coral::IAuthenticationCredentials::userItem());
            std::string password = iRoleEntry->second->valueForItem(coral::IAuthenticationCredentials::passwordItem());
            coral::LFCReplicaService::ReplicaDescription* description =
              new coral::LFCReplicaService::ReplicaDescription( pcs,
                                                                authenticationMethod,
                                                                userName,
                                                                password,
                                                                host,
                                                                coral::LFCReplicaService::On,
                                                                accessMode);
            replicaSet->appendReplica(lcs,role,description);
          }
          if(iAuthSet->second->hasDefault() && iAuthSet->second->credentials().numberOfItems()>0) {
            try{
              std::string userName = iAuthSet->second->credentials().valueForItem(coral::IAuthenticationCredentials::userItem());
              std::string password = iAuthSet->second->credentials().valueForItem(coral::IAuthenticationCredentials::passwordItem());
              coral::LFCReplicaService::ReplicaDescription* description=
                new coral::LFCReplicaService::ReplicaDescription( pcs,
                                                                  authenticationMethod,
                                                                  userName,
                                                                  password,
                                                                  host,
                                                                  coral::LFCReplicaService::On,
                                                                  accessMode);
              replicaSet->appendReplica(lcs,coral::IAuthenticationCredentials::defaultRole(),description);
            } catch (coral::Exception& e) {
              delete replicaSet;
              cleanUpData(authData,lookupData);
              coral::MessageStream log( m_serviceCallerName );
              log << coral::Error <<"Failure processing connection="<<pcs<<": could not find credentials."<<coral::MessageStream::endmsg;
              throw coral::Exception("Could not find credential parameters: "+std::string(e.what()),
                                     "XMLReplicaImporter::getData","LFCReplicaService");
            }


          }

        } else {
          delete replicaSet;
          cleanUpData(authData,lookupData);
          coral::MessageStream log( m_serviceCallerName );
          log << coral::Error <<"Failure processing connection="<<pcs<<": no authentication credential found."<<coral::MessageStream::endmsg;
          throw coral::Exception("Could not find credentials.",
                                 "XMLReplicaImporter::getData",
                                 "LFCReplicaService");
        }
      } else {
        if(authenticationMethod.empty()) authenticationMethod = "None";
        coral::LFCReplicaService::ReplicaDescription* description=
          new coral::LFCReplicaService::ReplicaDescription( pcs,
                                                            authenticationMethod,
                                                            "",
                                                            "",
                                                            host,
                                                            coral::LFCReplicaService::On,
                                                            accessMode);
        replicaSet->appendReplica(lcs,coral::IAuthenticationCredentials::defaultRole(),description);
      }

    }
  }
  cleanUpData(authData,lookupData);
  return replicaSet;
}
