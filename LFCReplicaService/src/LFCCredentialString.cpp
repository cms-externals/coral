#include "LFCCredentialString.h"
#include "CoralCommon/Cipher.h"

#include <cstdio>

std::string
coral::LFCReplicaService::LFCCredentialString::key(const std::string& guid){

  std::string tmp("");
  size_t j=guid.length()-2;
  for(size_t i=0; i<guid.length(); i+=2) {
    if(guid[i]=='-') i++;
    if(guid[j+1]=='-') j--;
    unsigned int first =0;
    ::sscanf( guid.substr(i,2).c_str(),"%x",&first );
    unsigned int last =0;
    char lch[3]={guid[j+1],guid[j],0};
    ::sscanf( lch,"%x",&last );
    char val = (char)((first^last)|1);
    tmp.append(1,val);
    j-=2;
  }
  return coral::Cipher::u64enc(tmp);
}

std::string
coral::LFCReplicaService::LFCCredentialString::password(const std::string& credentialString,
                                                        const std::string& guid){
  return coral::Cipher::decode(credentialString,key(guid));
}

std::string
coral::LFCReplicaService::LFCCredentialString::credentials(const std::string& password,
                                                           const std::string& guid){
  return coral::Cipher::encode(password,key(guid));
}
