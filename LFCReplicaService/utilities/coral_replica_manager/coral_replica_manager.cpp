#include <iostream>
#include <sstream>
#include <fstream>
#include "CoralBase/Exception.h"
#include "CoralCommon/CommandLine.h"
#include "CoralCommon/URIParser.h"
#include "CoralCommon/URIException.h"
#include "../../src/LFCReplicaService.h"
#include "../../src/ReplicaSet.h"
#include "../../src/XMLReplicaImporter.h"
#include "../../src/ReplicaFilter.h"


int main (int argc, char** argv)
{
  std::vector<coral::Option> secondaryOptions;
  //
  coral::Option lcsPar("lcs");
  lcsPar.flag = "-l";
  lcsPar.helpEntry = "the logical connection string";
  lcsPar.type = coral::Option::STRING;
  secondaryOptions.push_back(lcsPar);
  //
  coral::Option pcsPar("pcs");
  pcsPar.flag = "-c";
  pcsPar.helpEntry = "the physical connection string";
  pcsPar.type = coral::Option::STRING;
  secondaryOptions.push_back(pcsPar);
  //
  coral::Option rolePar("role");
  rolePar.flag = "-r";
  rolePar.helpEntry ="the role associated to the credentials";
  rolePar.type = coral::Option::STRING;
  secondaryOptions.push_back(rolePar);
  //
  coral::Option groupPar("group");
  groupPar.flag = "-g";
  groupPar.helpEntry ="the group name associated to the grid role";
  groupPar.type = coral::Option::STRING;
  secondaryOptions.push_back(groupPar);
  //
  coral::Option hostPar("host");
  hostPar.flag = "-h";
  hostPar.helpEntry = "the database host name";
  hostPar.type = coral::Option::STRING;
  secondaryOptions.push_back(hostPar);
  //
  coral::Option userPar("username");
  userPar.flag = "-u";
  userPar.helpEntry = "the username of the database account";
  userPar.type = coral::Option::STRING;
  secondaryOptions.push_back(userPar);
  //
  coral::Option pwdPar("password");
  pwdPar.flag = "-p";
  pwdPar.helpEntry = "the password of the database account";
  pwdPar.type = coral::Option::STRING;
  secondaryOptions.push_back(pwdPar);
  //
  coral::Option roOpt("read_only_mode");
  roOpt.flag = "-ro";
  roOpt.helpEntry = "read-only access mode for the replica";
  roOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(roOpt);
  //
  coral::Option woOpt("write_only_mode");
  woOpt.flag = "-wo";
  woOpt.helpEntry = "write-only access mode for the replica";
  woOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(woOpt);
  //
  coral::Option rwOpt("read_write_mode");
  rwOpt.flag = "-rw";
  rwOpt.helpEntry = "read-write access mode for the replica";
  rwOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(rwOpt);
  //
  coral::Option noAccessOpt("no_access_mode");
  noAccessOpt.flag = "-na";
  noAccessOpt.helpEntry = "no access allowed to the replica";
  noAccessOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(noAccessOpt);
  //
  coral::Option statOnOpt("status_on");
  statOnOpt.flag = "-on";
  statOnOpt.helpEntry = "status on for the replica database server";
  statOnOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(statOnOpt);
  //
  coral::Option statOffOpt("status_off");
  statOffOpt.flag = "-off";
  statOffOpt.helpEntry = "status off for the replica database server";;
  statOffOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(statOffOpt);
  //
  coral::Option lfFilePar("lookup_file");
  lfFilePar.flag = "-lf";
  lfFilePar.helpEntry = "the lookup service file";
  lfFilePar.type = coral::Option::STRING;
  secondaryOptions.push_back(lfFilePar);
  //
  coral::Option afFilePar("authentication_file");
  afFilePar.flag = "-af";
  afFilePar.helpEntry = "the authentication service file";
  afFilePar.type = coral::Option::STRING;
  secondaryOptions.push_back(afFilePar);
  //
  coral::Option noAuthOpt("no_authentication");
  noAuthOpt.flag = "-np";
  noAuthOpt.helpEntry = "no authentication required";
  noAuthOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(noAuthOpt);
  //
  coral::Option scanFolderOpt("scan_folder");
  scanFolderOpt.flag = "-s";
  scanFolderOpt.helpEntry = "scan child folders";
  scanFolderOpt.type = coral::Option::BOOLEAN;
  secondaryOptions.push_back(scanFolderOpt);
  //
  std::vector<coral::Command> mainSet;
  //
  coral::Command addRep("add");
  addRep.flag = "-add";
  addRep.helpEntry = "adding a new replica entry";
  addRep.type = coral::Option::BOOLEAN;
  addRep.exclusive = true;
  addRep.addOption(lcsPar.name,true);
  addRep.addOption(pcsPar.name,true);
  addRep.addOption(rolePar.name);
  addRep.addOption(hostPar.name);
  addRep.addOption(userPar.name);
  addRep.addOption(pwdPar.name);
  addRep.addOption(roOpt.name);
  addRep.addOption(rwOpt.name);
  addRep.addOption(noAuthOpt.name);
  mainSet.push_back(addRep);
  //
  coral::Command listRep("list");
  listRep.flag = "-ls";
  listRep.helpEntry = "listing the replica entries";
  listRep.type = coral::Option::BOOLEAN;
  listRep.exclusive = true;
  listRep.addOption(lcsPar.name);
  listRep.addOption(pcsPar.name);
  listRep.addOption(rolePar.name);
  listRep.addOption(hostPar.name);
  listRep.addOption(userPar.name);
  listRep.addOption(roOpt.name);
  listRep.addOption(rwOpt.name);
  listRep.addOption(statOnOpt.name);
  listRep.addOption(statOffOpt.name);
  listRep.addOption(noAuthOpt.name);
  listRep.addOption(scanFolderOpt.name);
  mainSet.push_back(listRep);
  //
  coral::Command setRep("set_param");
  setRep.flag = "-set";
  setRep.helpEntry = "set a replica parameter";
  setRep.type = coral::Option::BOOLEAN;
  setRep.exclusive = true;
  setRep.addOption(hostPar.name,true);
  setRep.addOption(userPar.name);
  setRep.addOption(pwdPar.name);
  setRep.addOption(statOnOpt.name);
  setRep.addOption(statOffOpt.name);
  mainSet.push_back(setRep);
  //
  coral::Command delRep("delete");
  delRep.flag = "-del";
  delRep.helpEntry = "delete replica entries";
  delRep.type = coral::Option::BOOLEAN;
  delRep.exclusive = true;
  delRep.addOption(lcsPar.name,true);
  delRep.addOption(pcsPar.name);
  delRep.addOption(rolePar.name);
  delRep.addOption(hostPar.name);
  delRep.addOption(userPar.name);
  delRep.addOption(roOpt.name);
  delRep.addOption(rwOpt.name);
  delRep.addOption(statOnOpt.name);
  delRep.addOption(statOffOpt.name);
  delRep.addOption(noAuthOpt.name);
  mainSet.push_back(delRep);
  //
  coral::Command valRep("validate_param");
  valRep.flag = "-val";
  valRep.helpEntry = "validate a replica parameter";
  valRep.type = coral::Option::BOOLEAN;
  valRep.exclusive = true;
  valRep.addOption(hostPar.name,true);
  valRep.addOption(userPar.name,true);
  valRep.addOption(pwdPar.name,true);
  mainSet.push_back(valRep);
  //
  coral::Command setPermCmd("set_permissions");
  setPermCmd.flag = "-set_perm";
  setPermCmd.helpEntry = "set access permission for a group";
  setPermCmd.type = coral::Option::BOOLEAN;
  setPermCmd.exclusive = true;
  setPermCmd.addOption(lcsPar.name,true);
  setPermCmd.addOption(rolePar.name,true);
  setPermCmd.addOption(groupPar.name,true);
  setPermCmd.addOption(roOpt.name);
  setPermCmd.addOption(woOpt.name);
  setPermCmd.addOption(rwOpt.name);
  setPermCmd.addOption(noAccessOpt.name);
  mainSet.push_back(setPermCmd);
  //
  coral::Command expCmd("export");
  expCmd.flag = "-exp";
  expCmd.helpEntry = "export replica data set";
  expCmd.type = coral::Option::BOOLEAN;
  expCmd.exclusive = true;
  expCmd.addOption(lcsPar.name);
  expCmd.addOption(pcsPar.name);
  expCmd.addOption(rolePar.name);
  expCmd.addOption(hostPar.name);
  expCmd.addOption(userPar.name);
  expCmd.addOption(roOpt.name);
  expCmd.addOption(rwOpt.name);
  expCmd.addOption(statOnOpt.name);
  expCmd.addOption(statOffOpt.name);
  expCmd.addOption(noAuthOpt.name);
  expCmd.addOption(scanFolderOpt.name);
  expCmd.addOption(lfFilePar.name);
  expCmd.addOption(afFilePar.name);
  mainSet.push_back(expCmd);
  //
  coral::Command impCmd("import");
  impCmd.flag = "-imp";
  impCmd.type = coral::Option::BOOLEAN;
  impCmd.helpEntry ="import replica data set";
  impCmd.exclusive = true;
  impCmd.addOption(lfFilePar.name);
  impCmd.addOption(afFilePar.name);
  mainSet.push_back(impCmd);
  //
  try{
    std::string lcs("");
    std::string pcs("");
    std::string host("");
    std::string userName("");
    std::string groupName("");
    std::string password("");
    std::string role("");
    std::string lookupFile("dblookup.xml");
    std::string authenticationFile("authentication.xml");
    std::string authenticationMethod("None");
    bool readOnlyMode = false;
    bool writeOnlyMode = false;
    bool updateMode = false;
    bool noaccess = false;
    std::string prevAccessMode("");
    bool statusOn = false;
    bool statusOff = false;
    bool passwordAuthentication = true;
    bool scanFolder = false;
    coral::CommandLine cmd(secondaryOptions,mainSet);
    cmd.parse(argc,argv);
    const std::map<std::string,std::string>& ops = cmd.userOptions();
    if(cmd.userCommand()==coral::CommandLine::helpOption().name || ops.size()==0) {
      cmd.help(std::cout);
      return 0;
    } else {
      std::map<std::string,std::string>::const_iterator iO=ops.find(coral::CommandLine::helpOption().name);
      if(iO!=ops.end()) {
        cmd.help(cmd.userCommand(),std::cout);
        return 0;
      } else {
        iO = ops.find(lcsPar.name);
        if(iO!=ops.end()) lcs = iO->second;
        iO = ops.find(pcsPar.name);
        if(iO!=ops.end()) pcs = iO->second;
        iO = ops.find(rolePar.name);
        if(iO!=ops.end()) role = iO->second;
        iO = ops.find(groupPar.name);
        if(iO!=ops.end()) groupName = iO->second;
        iO = ops.find(hostPar.name);
        if(iO!=ops.end()) host = iO->second;
        iO = ops.find(userPar.name);
        if(iO!=ops.end()) userName = iO->second;
        iO = ops.find(noAuthOpt.name);
        if(iO!=ops.end()) passwordAuthentication = false;
        iO = ops.find(pwdPar.name);
        if(iO!=ops.end()) password =  iO->second;
        if(!passwordAuthentication) {
          if(!userName.empty()) throw coral::ExclusiveOptionException(userPar.name,noAuthOpt.name);
          if(!password.empty()) throw coral::ExclusiveOptionException(pwdPar.name,noAuthOpt.name);
        } else {
          authenticationMethod = "password";
        }
        iO = ops.find(lfFilePar.name);
        if(iO!=ops.end()) lookupFile = iO->second;
        iO = ops.find(afFilePar.name);
        if(iO!=ops.end()) authenticationFile = iO->second;
        iO = ops.find(roOpt.name);
        if(iO!=ops.end()) {
          readOnlyMode = true;
          prevAccessMode = roOpt.name;
        }
        iO = ops.find(woOpt.name);
        if(iO!=ops.end()) {
          writeOnlyMode = true;
          if(prevAccessMode.empty()) {
            prevAccessMode = woOpt.name;
          } else {
            throw coral::ExclusiveOptionException(prevAccessMode,woOpt.name);
          }
        }
        iO = ops.find(rwOpt.name);
        if(iO!=ops.end()) {
          updateMode = true;
          if(prevAccessMode.empty()) {
            prevAccessMode = rwOpt.name;
          } else {
            throw coral::ExclusiveOptionException(prevAccessMode,rwOpt.name);
          }
        }
        iO = ops.find(noAccessOpt.name);
        if(iO!=ops.end()) {
          noaccess  = true;
          if(prevAccessMode.empty()) {
            prevAccessMode = noAccessOpt.name;
          } else {
            throw coral::ExclusiveOptionException(prevAccessMode,noAccessOpt.name);
          }
        }
        iO = ops.find(statOnOpt.name);
        if(iO!=ops.end()) statusOn = true;
        iO = ops.find(statOffOpt.name);
        if(iO!=ops.end()) statusOff = true;
        if(statusOn && statusOff) {
          throw coral::ExclusiveOptionException(statOnOpt.name,statOffOpt.name);
        }
        iO = ops.find(scanFolderOpt.name);
        if(iO!=ops.end()) scanFolder = true;

        coral::LFCReplicaService::LFCReplicaService replicaService("CORAL/Services/LFCReplicaService");

        coral::AccessMode accessMode = coral::Update;
        if(readOnlyMode) accessMode = coral::ReadOnly;
        coral::LFCReplicaService::ReplicaStatus status = coral::LFCReplicaService::On;
        if( statusOff ) status = coral::LFCReplicaService::Off;

        if(cmd.userCommand()==addRep.name) {
          if(passwordAuthentication) {
            if(userName.empty()) throw coral::MissingRequiredOptionException(userPar.name);
            if(password.empty()) throw coral::MissingRequiredOptionException(pwdPar.name);
          }
          if(host.empty()) {
            try{
              coral::URIParser parser;
              parser.setURI(pcs);
              host = parser.hostName();
            } catch (const coral::URIException& e) {
              throw coral::Exception("Could not derive host name from connection string: "+std::string(e.what()),
                                     "coral_replica_manager -add","LFCReplicaService");
            }
          }
          coral::LFCReplicaService::ReplicaDescription description(pcs,
                                                                   authenticationMethod,
                                                                   userName,
                                                                   password,
                                                                   host,
                                                                   coral::LFCReplicaService::On,
                                                                   accessMode);
          replicaService.addReplica(lcs,role,description);
          return 0;
        }
        if(cmd.userCommand()==listRep.name) {
          coral::LFCReplicaService::ReplicaFilter filter;
          if(!passwordAuthentication) filter.setAuthenticationMechanism(authenticationMethod);
          if(!userName.empty()) filter.setUserName(userName);
          if(!host.empty()) filter.setServerName(host);
          if(readOnlyMode || updateMode) filter.setAccessMode(accessMode);
          if(statusOn || statusOff) filter.setStatus(status);
          coral::LFCReplicaService::ReplicaSet* replicaSet = replicaService.listReplicas(lcs,role,filter,scanFolder);
          replicaSet->prettyPrint(std::cout);
          delete replicaSet;
          return 0;
        }
        if(cmd.userCommand()==setRep.name) {
          if(!userName.empty() && !statusOn && !statusOff) {
            if(password.empty()) throw coral::MissingRequiredOptionException(pwdPar.name);
          }
          if(!password.empty()) {
            if(statusOn) throw coral::ExclusiveOptionException(pwdPar.name,statOnOpt.name);
            if(statusOff) throw coral::ExclusiveOptionException(pwdPar.name,statOffOpt.name);
            if(userName.empty()) throw coral::MissingRequiredOptionException(userPar.name);
            replicaService.setReplicaPassword(host,userName,password );
          } else {
            replicaService.setReplicaStatus(host,userName,status);
          }
          return 0;
        }
        if(cmd.userCommand()==delRep.name) {
          coral::LFCReplicaService::ReplicaFilter filter;
          if(!passwordAuthentication) filter.setAuthenticationMechanism(authenticationMethod);
          if(!userName.empty()) filter.setUserName(userName);
          if(!host.empty()) filter.setServerName(host);
          if(readOnlyMode || updateMode) filter.setAccessMode(accessMode);
          if(statusOn || statusOff) filter.setStatus(status);
          if(!pcs.empty()) filter.setPhysicalConnectionString(pcs);
          replicaService.deleteReplicas(lcs,role,filter);
          return 0;
        }
        if(cmd.userCommand()==valRep.name) {
          replicaService.verifyReplicaPassword(host,userName,password );
          return 0;
        }
        if(cmd.userCommand()==setPermCmd.name) {
          if(!readOnlyMode && !updateMode && !writeOnlyMode && !noaccess) {
            throw coral::OptionException("Permission mode has not been specified");
          }
          coral::LFCReplicaService::ReplicaPermission permission = coral::LFCReplicaService::NoPerm;
          if(noaccess) {
            permission = coral::LFCReplicaService::NoPerm;
          }
          if(readOnlyMode) {
            permission = coral::LFCReplicaService::RPerm;
          }
          if(writeOnlyMode) {
            permission = coral::LFCReplicaService::WPerm;
          }
          if(updateMode) {
            permission = coral::LFCReplicaService::RWPerm;
          }
          replicaService.setReplicaAccessPermission(lcs,role,groupName,permission);
        }
        if(cmd.userCommand()==expCmd.name) {
          coral::LFCReplicaService::ReplicaFilter filter;
          if(!passwordAuthentication) filter.setAuthenticationMechanism(authenticationMethod);
          if(!userName.empty()) filter.setUserName(userName);
          if(!host.empty()) filter.setServerName(host);
          if(readOnlyMode || updateMode) filter.setAccessMode(accessMode);
          if(statusOn || statusOff) filter.setStatus(status);
          coral::LFCReplicaService::ReplicaSet* replicaSet = replicaService.listReplicas(lcs,role,filter,scanFolder);
          std::ofstream lookupFileStream(lookupFile.c_str(),std::ios_base::out);
          std::ofstream authenticationFileStream(authenticationFile.c_str(),std::ios_base::out);
          replicaSet->XMLPrint(lookupFileStream,authenticationFileStream);
          lookupFileStream.close();
          authenticationFileStream.close();
          delete replicaSet;
          return 0;
        }
        if(cmd.userCommand()==impCmd.name) {
          coral::LFCReplicaService::XMLReplicaImporter importer("coral_replica_manager",lookupFile,authenticationFile);
          coral::LFCReplicaService::ReplicaSet* replicaSet = importer.getData();
          if( replicaSet) {
            replicaService.importReplicas( *replicaSet );
            delete replicaSet;
          }
          return 0;
        }
      }
      return 0;
    }
  } catch (const std::exception& e) {
    std::cout << "ERROR: " << e.what() << std::endl;
    return -1;
  }
}
