// -*- C++ -*-
// $Id: MultipleSchemasApp.h,v 1.5 2011/03/22 10:29:55 avalassi Exp $
#ifndef MYSQLACCESS_CONNECTIONAPP_H
#define MYSQLACCESS_CONNECTIONAPP_H 1

#include "../Common/TestBase.h"

class MultipleSchemasApp : virtual public TestBase
{
public:
  MultipleSchemasApp( const std::string& connectionString, const std::string& userName, const std::string& password, const std::string& otherSchema, coral::AccessMode );
  virtual ~MultipleSchemasApp();
  void run();

private:
  std::string m_connectionString;
  std::string m_userName;
  std::string m_password;
  std::string m_otherSchema;
  coral::AccessMode m_accessMode;
};

#endif // MYSQLACCESS_CONNECTIONAPP_H
