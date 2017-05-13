// -*- C++ -*-
// $Id: ConnectionApp.h,v 1.3 2011/03/22 10:29:55 avalassi Exp $
#ifndef MYSQL_CONNECTIONAPP_H
#define MYSQL_CONNECTIONAPP_H 1

#include <string>

class ConnectionApp
{
public:
  ConnectionApp( const std::string& connectionString, const std::string& userName, const std::string& password );
  virtual ~ConnectionApp();
  void run();

public:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
};

#endif // MYSQL_CONNECTIONAPP_H
