#include "Config.h"

using namespace coral;

//-----------------------------------------------------------------------------

const Config& coral::getConfig() {
  static Config config;
  return config;
}

//-----------------------------------------------------------------------------

const std::string& Config::password() const
{
  static std::string password="testpw";
  return password;
}

//-----------------------------------------------------------------------------

const std::string& Config::dbConnectionString() const
{
  static std::string dbConnectionString="sqlite_file:authtest.db";
  return dbConnectionString;
}
