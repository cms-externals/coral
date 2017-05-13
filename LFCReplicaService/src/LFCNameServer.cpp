#include "LFCNameServer.h"
#include "LFCReplica.h"
#include "Guid.h"
#include <serrno.h>
#include <sys/stat.h> //mode_t in lfc_api.h
#include <lfc_api.h>
#include "LFCException.h"
#include <cstdlib> // getenv
#include <cstring> // strcpy

coral::LFCReplicaService::LFCNameServer::LFCNameServer() : m_transactionActive(false){
}

coral::LFCReplicaService::LFCNameServer::~LFCNameServer(){
}

void coral::LFCReplicaService::LFCNameServer::startTransaction(const std::string& message){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if(m_transactionActive) throw LFCException("Transaction is already active.");
  m_transactionActive = (lfc_starttrans(lfcHost, const_cast<char*>(message.c_str())) == 0);
  if(!m_transactionActive) throw LFCException("Could not start the transaction.","lfc_starttrans");
}

void coral::LFCReplicaService::LFCNameServer::commitTransaction(){
  if(!m_transactionActive) throw LFCException("Transaction is not active.");
  if(lfc_endtrans()==0) {
    m_transactionActive = false;
  } else {
    throw LFCException("Could not commit the transaction.","lfc_endtrans");
  }
}

void coral::LFCReplicaService::LFCNameServer::abortTransaction(){
  if(m_transactionActive) {
    if(lfc_aborttrans()==0) {
      m_transactionActive = false;
    } else {
      throw LFCException("Could not abort the transaction.","lfc_aborttrans");
    }
  }
}

bool coral::LFCReplicaService::LFCNameServer::isTransactionActive(){
  return m_transactionActive;
}

void coral::LFCReplicaService::LFCNameServer::makeDir(const std::string& absolutePath){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if(lfc_mkdir(absolutePath.c_str(),0750)<0) {
    throw LFCException("Could not create folder \""+absolutePath,"lfc_mkdir");
  }
}

void coral::LFCReplicaService::LFCNameServer::rmDir(const std::string& absolutePath){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if(lfc_rmdir(absolutePath.c_str())<0) {
    throw LFCException("Could not remove folder \""+absolutePath,"lfc_rmdir");
  }
}

std::string coral::LFCReplicaService::LFCNameServer::makeFile(const std::string& absolutePath){
  Guid newGuid;
  Guid::create(newGuid);
  std::string lfnGuid = newGuid.toString();
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if (lfc_creatg(absolutePath.c_str(), lfnGuid.c_str(),0700) < 0) {
    throw LFCException("Cannot add the logical connection string to LFC Database.","lfc_creatg");
  }
  return lfnGuid;
}

void coral::LFCReplicaService::LFCNameServer::setPermissionToGroup(const std::string& absolutePath,
                                                                   const std::string& groupName,
                                                                   int mode){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  gid_t groupId = 0;
  std::string gName(groupName);
  if (lfc_getgrpbynam(const_cast<char*>(gName.c_str()),&groupId) < 0) {
    throw LFCException("Cannot resolve group Id.","lfc_getgrpbyname");
  }
  int nentries = 0;
  nentries = lfc_getacl(absolutePath.c_str(),0,NULL);
  if (nentries < 0) {
    throw LFCException("Cannot get the number of acl entries for the specified file.","lfc_getacl");
  }
  lfc_acl* aclEntries = static_cast<lfc_acl*>(::calloc(nentries+2,sizeof(lfc_acl)));
  if(nentries >0) {
    if (lfc_getacl(absolutePath.c_str(),nentries,aclEntries) < 0) {
      ::free(aclEntries);
      throw LFCException("Cannot get the acl entries for the specified file.","lfc_setacl");
    }
  }
  lfc_acl*  aclForGroup = 0;
  lfc_acl* aclMask = 0;
  for(int i=0; i<nentries; i++) {
    if((aclEntries[i].a_type == CNS_ACL_GROUP) && (aclEntries[i].a_id==int(groupId))) {
      aclForGroup = &aclEntries[i];
      break;
    }
  }
  for(int i=0; i<nentries; i++) {
    if(int(aclEntries[i].a_type) == CNS_ACL_MASK) {
      aclMask = &aclEntries[i];
      break;
    }
  }
  if(!aclForGroup) {
    aclForGroup = &aclEntries[nentries];
    ++nentries;
  }
  if(!aclMask) {
    aclMask = &aclEntries[nentries];
    aclMask->a_perm = 0;
    ++nentries;
  }
  aclForGroup->a_type = CNS_ACL_GROUP;
  aclForGroup->a_id = groupId;
  aclForGroup->a_perm = mode;
  aclMask->a_type = CNS_ACL_MASK;
  aclMask->a_id = 0;
  aclMask->a_perm |= 5;
  if (lfc_setacl(absolutePath.c_str(),nentries,aclEntries) < 0) {
    ::free(aclEntries);
    throw LFCException("Cannot set acl to the specified file.","lfc_setacl");
  }
  ::free(aclEntries);
}

int coral::LFCReplicaService::LFCNameServer::getPermissionToGroup(const std::string& absolutePath,
                                                                  const std::string& groupName){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  gid_t groupId = 0;
  std::string gName(groupName);
  if (lfc_getgrpbynam(const_cast<char*>(gName.c_str()),&groupId) < 0) {
    throw LFCException("Cannot resolve group Id.","lfc_getgrpbyname");
  }
  int nentries = 0;
  nentries = lfc_getacl(absolutePath.c_str(),0,NULL);
  if (nentries < 0) {
    throw LFCException("Cannot get the number of acl entries for the specified file.","lfc_getacl");
  }
  int perm = 0;
  if(nentries >0) {
    lfc_acl* aclEntries = static_cast<lfc_acl*>(::calloc(nentries,sizeof(lfc_acl)));
    if (lfc_getacl(absolutePath.c_str(),nentries,aclEntries) < 0) {
      ::free(aclEntries);
      throw LFCException("Cannot get the acl entries for the specified file.","lfc_setacl");
    }
    for(int i=0; i<nentries; i++) {
      if((aclEntries[i].a_type == CNS_ACL_GROUP) && (aclEntries[i].a_id==int(groupId))) {
        perm |= aclEntries[i].a_perm;
      }
    }
    ::free(aclEntries);
  }
  return perm;
}

void coral::LFCReplicaService::LFCNameServer::rmFile(const std::string& absolutePath){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if(lfc_unlink(absolutePath.c_str())<0) {
    throw LFCException("Could not remove file \""+absolutePath+"\"","lfc_unlink");
  }
}

void coral::LFCReplicaService::LFCNameServer::addReplica(const std::string& guid,
                                                         const LFCReplica& replica){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if (lfc_addreplica(guid.c_str(),
                     NULL,
                     replica.host.c_str(),
                     replica.sfn.c_str(),
                     replica.status,
                     replica.f_type,
                     replica.poolname.c_str(),
                     replica.fs.c_str()) < 0 ) {
    throw LFCException("Could not add the replica.","lfc_addreplica");
  }
}

void
coral::LFCReplicaService::LFCNameServer::addReplica(const LFCReplica& replica)
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  lfc_fileid fid;
  ::strcpy( fid.server, replica.host.c_str() );
  //fid.fileid = replica.fileid.id;
  if (lfc_addreplica(NULL,
                     &fid,
                     replica.host.c_str(),
                     replica.sfn.c_str(),
                     replica.status,
                     replica.f_type,
                     replica.poolname.c_str(),
                     replica.fs.c_str()) < 0 ) {
    throw LFCException("Could not add the replica.","lfc_addreplica");
  }
}
/*
const coral::LFCReplicaService::LFCReplicaSet*
coral::LFCReplicaService::LFCNameServer::listReplicas( const std::string& absolutePath ) const
{
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost)
    throw LFCException("LFC host has not been defined");
  //create a new replicma set
  LFCReplicaSet* replicaSet = new LFCReplicaSet;
  //array of replicas
  lfc_filereplica* replicas;
  int entries = 0;  //the amount of replicas
  //get all replicas in one go
  int ret = lfc_getreplica( absolutePath.c_str(), 0, 0, &entries, &replicas );
  //check the response
  if( ret == -1 )
  //some errors happen
    throw LFCException( sstrerror(serrno) );
  //iterate all replicas
  lfc_filereplica* replica = replicas;
  for( int i = 0; i < entries; i++ )
  {
    //create a new replica for the vector
    LFCReplica* rep = new LFCReplica;
    //create a file
    LFCFileId fid;
    //do the copy stuff
    fid.id = replica->fileid;
    fid.lfcHost = lfcHost;
    rep->fileid = fid;
    rep->pfn = replica->sfn;
    rep->status = replica->status;
    rep->serverParameter = replica->host;
    rep->f_typeParameter = replica->f_type;
    rep->poolnameParameter = replica->poolname;
    rep->fsParameter = replica->fs;
    //add the replica to the vector
    replicaSet->replicas().push_back( rep );
    //increase to the next replica of the array
    replica++;
  }
  //destroy the replicas
  free( replicas );
  //and return
  return replicaSet;
}

coral::LFCReplicaService::LFCReplicaSet*
coral::LFCReplicaService::LFCNameServer::listReplicas( const std::string& poolnameParameter,
                                                       const std::string& serverParameter,
                                                       const std::string& fsParameter){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  LFCReplicaSet* replicaSet = new LFCReplicaSet;
  struct lfc_filereplica *lfcFileReplica;
        lfc_list lfcList;
  int flags = CNS_LIST_BEGIN;
  const char* poolname = (poolnameParameter.empty()?NULL:poolnameParameter.c_str());
  const char* server = (serverParameter.empty()?NULL:serverParameter.c_str());
  const char* fs = (fsParameter.empty()?NULL:fsParameter.c_str());
  //lfc_listreplicax( const char* poolname, const char* server, const char* fs, int flags, lfc_list* listp)
  //lfc_getreplicax( const char* path, const char* guid, const char* se, int* nbentries, struct lfc_filereplicax** rep_entries)


  while((lfcFileReplica = lfc_listreplicax(poolname,
                                           server,
                                           fs,
                                           flags,
                                           &lfcList)) != NULL){
    LFCReplica* replica = new LFCReplica;
    LFCFileId fid;
    fid.id = lfcFileReplica->fileid;
    fid.lfcHost = lfcHost;
    replica->fileid = fid;
    replica->pfn = lfcFileReplica->sfn;
    replica->status = lfcFileReplica->status;
    replica->serverParameter = lfcFileReplica->host;
    replica->f_typeParameter = lfcFileReplica->f_type;
    replica->poolnameParameter = lfcFileReplica->poolname;
    replica->fsParameter = lfcFileReplica->fs;
    replicaSet->replicas().push_back(replica);

    flags=CNS_LIST_CONTINUE;
  }

  // Final call to terminate list and free the resources. Refer lfc_listreplica man pages.
  (void) lfc_listreplicax(poolname,
                         server,
                         fs,
                         CNS_LIST_END,
                         &lfcList);
  return replicaSet;
}
*/
size_t coral::LFCReplicaService::LFCNameServer::deleteReplicas( const std::string& absolutePath ){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  struct lfc_filestatg statg;
  if (lfc_statg(absolutePath.c_str(), NULL, &statg) < 0) {
    throw LFCException("Could not find logical file name.","lfc_statg");
  }

  struct lfc_filereplica *lfcFileReplica;
  lfc_list lfcList;
  int flags = CNS_LIST_BEGIN;
  size_t deleted = 0;
  while((lfcFileReplica = lfc_listreplica(absolutePath.c_str(), NULL, flags, &lfcList)) != NULL) {
    if (lfc_delreplica(statg.guid, NULL, lfcFileReplica->sfn) < 0 ) {
      throw LFCException("Could Not Delete the Replica: pcs="+std::string(lfcFileReplica->sfn),"lfc_delreplica");
    }
    ++deleted;
    flags=CNS_LIST_CONTINUE;
  }
  // Final call to terminate list and free the resources. Refer lfc_listreplica man pages.
  (void)   lfc_listreplica(absolutePath.c_str(),  NULL,  CNS_LIST_END, &lfcList);
  return deleted;
}

size_t coral::LFCReplicaService::LFCNameServer::deleteReplica( const std::string& absolutePath, const std::string& pfn ){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  struct lfc_filestatg statg;
  if (lfc_statg(absolutePath.c_str(), NULL, &statg) < 0) {
    throw LFCException("Could not find logical file name.","lfc_statg");
  }
  size_t deleted = 0;
  if (lfc_delreplica(statg.guid, NULL, pfn.c_str()) < 0 ) {
    throw LFCException("Could Not Delete the Replica: "+pfn,"lfc_delreplica");
  } else {
    ++deleted;
  }
  return deleted;
}

void coral::LFCReplicaService::LFCNameServer::deleteReplicaByGuid( const std::string& guid, const std::string& pfn ){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if (lfc_delreplica(guid.c_str(), NULL, pfn.c_str()) < 0 ) {
    throw LFCException("Could Not Delete the Replica: "+pfn,"lfc_delreplica");
  }
}

void coral::LFCReplicaService::LFCNameServer::deleteReplicaByFileId( const LFCFileId& fileId, const std::string& pfn ){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  lfc_fileid fid;
  ::strcpy(fid.server,fileId.lfcHost.c_str());
  fid.fileid = fileId.id;
  if (lfc_delreplica(NULL,&fid, pfn.c_str()) < 0 ) {
    throw LFCException("Could Not Delete the Replica: "+pfn,"lfc_delreplica");
  }
}

void coral::LFCReplicaService::LFCNameServer::setReplicaStatus( const std::string& pfn, const std::string& status){
  char* lfcHost = ::getenv("LFC_HOST");
  if(!lfcHost) throw LFCException("LFC host has not been defined");
  if(!status.empty()) {
    if(lfc_setrstatus(pfn.c_str(),status[0]) < 0) {
      throw LFCException("Could not change status of replica","lfc_setrstatus");
    }
  }
}

const std::string
coral::LFCReplicaService::LFCNameServer::lfcHost() const
{
  std::string lfcHost("");
  char* lfcHostVar = ::getenv("LFC_HOST");
  if(lfcHostVar) lfcHost = std::string(lfcHostVar);
  return lfcHost;
}
