#include "ConnectionString.h"
#include <sstream>

///  parses the connection string and extract the service part
std::string coral::ConnectionService::ConnectionString::serviceConnectionString(const std::string& connectionString){
  // take the first bit of the connection string...
  std::string srvConnStr("");
  std::string::size_type pos = connectionString.rfind("/");
  if( (pos!=std::string::npos) && !(pos==connectionString.size()-1) )
  {
    srvConnStr = connectionString.substr( 0, pos );
  }
  return srvConnStr;
}


///  parses the connection string and extract the nominal schema
std::string coral::ConnectionService::ConnectionString::nominalSchema(const std::string& connectionString){
  // take the last bit of the connection string...
  std::string nomSchema("");
  std::string::size_type pos = connectionString.rfind("/");
  if( (pos!=std::string::npos) && !(pos==connectionString.size()-1) )
  {
    nomSchema =  connectionString.substr( pos+1 );
  }
  return nomSchema;
}

/// recompose the full connection string
std::string coral::ConnectionService::ConnectionString::connectionString(const std::string& serviceConnectionString,const std::string& nominalSchema){
  std::string connStr(serviceConnectionString);
  return connStr.append("/").append(nominalSchema);
}

/// compose a string from technology and service name
std::string
coral::ConnectionService::ConnectionString::serviceKey(const std::string& technologyName, const std::string& serviceName){
  std::ostringstream sk;
  sk << technologyName << ":" << serviceName;
  return sk.str();
}
