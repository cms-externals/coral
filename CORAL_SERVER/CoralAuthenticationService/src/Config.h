#ifndef CORAL_CONFIG_H
#define CORAL_CONFIG_H

#include <string>

namespace coral {

  class Config
  {
  public:
    Config() {};

    const std::string& password() const;

    const std::string& dbConnectionString() const;
  private:
    std::string m_password;

    std::string m_dbConnectionString;
  };

  const Config& getConfig();
}

#endif
