#include "LFCException.h"
#include <errno.h>
#include <serrno.h>
#include <sys/stat.h> //mode_t in lfc_api.h
#include <lfc_api.h>
#include <sstream>

coral::LFCReplicaService::LFCException::LFCException(const std::string& contextMessage) : coral::Exception("","",""),
                                                                                          m_message(contextMessage){
}

coral::LFCReplicaService::LFCException::LFCException(const std::string& contextMessage,
                                                     const std::string& lfcFunctionName) : coral::Exception("","",""),
                                                                                           m_message(""){
  std::ostringstream messageOs;
  messageOs << contextMessage;
  messageOs << " Function \"";
  messageOs << lfcFunctionName;
  messageOs << "\" returned error code=";
  messageOs << serrno;
  messageOs << " label=";
  messageOs << resolveCode(serrno);
  m_message = messageOs.str();
}

coral::LFCReplicaService::LFCException::LFCException(const std::string& contextMessage,
                                                     const std::string& lfcFunctionName,
                                                     int errorCode) : coral::Exception("","",""),
                                                                      m_message(""){
  std::ostringstream messageOs;
  messageOs << contextMessage;
  messageOs << " Function \"";
  messageOs << lfcFunctionName;
  messageOs << "\" returned error code=";
  messageOs << errorCode;
  messageOs << " label=";
  messageOs << resolveCode(errorCode);
  m_message = messageOs.str();
}

std::string coral::LFCReplicaService::LFCException::resolveCode(int code){
  std::string messageCode("UNRESOLVED");
  switch (code) {
  case SENOSHOST:
    messageCode = "SENOSHOST";
    break;
  case SENOSSERV:
    messageCode = "SENOSSERV";
    break;
  case SECOMERR:
    messageCode = "SECOMERR";
    break;
  case SEINTERNAL:
    messageCode = "SEINTERNAL";
    break;
  case ENSNACT:
    messageCode = "ENSNACT";
    break;
  case ENOENT:
    messageCode = "ENOENT";
    break;
  case EACCES:
    messageCode = "EACCES";
    break;
  case EFAULT:
    messageCode = "EFAULT";
    break;
  case EEXIST:
    messageCode = "EEXIST";
    break;
  case ENOTDIR:
    messageCode= "ENOTDIR";
    break;
  case ENOSPC:
    messageCode = "ENOSPC";
    break;
  case ENAMETOOLONG:
    messageCode = "ENAMETOOLONG";
    break;
  case EISDIR:
    messageCode = "EISDIR";
    break;
  case EINVAL:
    messageCode = "EINVAL";
    break;
  case EPERM:
    messageCode = "EPERM";
    break;
  case ENOMEM:
    messageCode = "ENOMEM";
    break;
  case ESEC_BAD_CREDENTIALS:
    messageCode = "ESEC_BAD_CREDENTIALS";
    break;
  case ESEC_BAD_MAGIC:
    messageCode = "ESEC_BAD_MAGIC";
    break;
  case ESEC_NO_USER:
    messageCode = "ESEC_NO_USER";
    break;
  case ESEC_NO_PRINC:
    messageCode = "ESEC_NO_PRINC";
    break;
  };
  return messageCode;
}
