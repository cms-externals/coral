// -*- C++ -*-
#ifndef SCHEMA_COPY_APP_H
#define SCHEMA_COPY_APP_H 1

#include "../Common/TestBase.h"

#include "RelationalAccess/ITableDescription.h"

#include <map>
#include <list>

class SchemaCopyApp : virtual public TestBase
{
public:
  struct TabDesc
  {
  public:
    TabDesc( const std::string& name, const coral::ITableDescription& tabdesc )
      : m_name( name )
      , m_desc( tabdesc )
    {
    }

    std::string m_name;
    const coral::ITableDescription& m_desc;
  };

public:
  SchemaCopyApp( const std::string& connectionString,
                 const std::string& source,
                 const std::string& dest,
                 const std::string& userName,
                 const std::string& password );
  virtual ~SchemaCopyApp();

  void readDbObjects();

  void cpDbObjects();

  void run();

private:
  // Connection params
  std::string m_connectionString;
  std::string m_source;
  std::string m_dest;
  std::string m_userName;
  std::string m_password;
  // Schema objects
  std::list<TabDesc> m_tables;
  // Schema copy session
  coral::ISession* m_session;
};

#endif
