#ifndef TESTCORE_TESTING_H
#define TESTCORE_TESTING_H 1

#include "TestEnv/SealSwitch.h"
#include "TestEnv/TestEnv.h"

#include "RelationalAccess/AccessMode.h"
#include "CoralCommon/Utilities.h"
#include "CoralBase/MessageStream.h"

namespace coral {
  class ITableDescription;
  class AttributeList;
  class ISessionProxy;
  class IConnectionService;
  class Blob;
}

class Testing
{
public:
  //constructor
  Testing(const TestEnv& env);
  //destructor
  virtual ~Testing();
  //create a session with coral::update mode
  void createSession(int index, coral::AccessMode mode = coral::Update, bool sharing = true);
  //create a session with a handle
  //    void createSession(int index, coral::IHandle<coral::IConnectionService> handle, coral::AccessMode mode);
  //delete all
  void deleteSession();
  //set session
  void setSession(coral::ISessionProxy * session);
  //checks with dbtype is set
  std::string getSchemaName() const;
  //    bool isDBType(const std::string& type);

  //create default tables T1, T2, T3
  void createSchemaDefault();
  //read from default tables
  void readSchemaDefault();
  //read from default tables
  void readSchemaDefault(std::string SchemaName);
  //create default view V0 based on default T1, T2, T3 tables
  //important: only use this function after calling createSchemaDefault()
  void createViewDefault();
  //fill default schema with values
  void fillSchemaDefault();
  //print default schema
  void printSchemaDefault();
  //create a simple table
  void createSimpleTable();

  template <class T1>
  void compareValues(const T1& source01, const T1& source02, const std::string& desc) const
  {
    if(source01 != source02)
    {
      throw std::runtime_error( "Unexpected value for " + desc );
    }
  }

  template <class T1>
  void compareValuesAndLog(const T1& source01, const T1& source02, const std::string& desc, coral::MessageStream& msgstream)
  {
    msgstream << coral::Info << desc << " [" << source01 << "]" << " ref[" << source02 << "]" << coral::MessageStream::endmsg;
    if(source01 != source02)
    {
      throw std::runtime_error( "value missmatch" );
    }
  }

  size_t decreaseValue(size_t orig, size_t value)
  {
    if( orig >= value )
    {
      return orig - value;
    }
    return 0;
  }

protected:
  //create a session with custom mode
  coral::ISessionProxy* newSession(int index, coral::AccessMode mode, bool sharing);
  //some tool functions
  void fillData(int i);

  void checkData(int i) const;

  void printTableInfo( const coral::ITableDescription& description ) const;

  void fillBlob(coral::Blob& blob, size_t maxc);

  void fillString(std::string& s, size_t maxc, size_t offset);

  coral::ISessionProxy& getSession()
  {
    if( !m_session )
      throw std::runtime_error( "No Session was started" );

    return *m_session;
  }

  coral::IConnectionService& getConnectionService();

  void writeTableT1(coral::ISessionProxy&, size_t numberOfFilledRows);

  size_t readTableT1(coral::ISessionProxy&);

  void writeTableT2(coral::ISessionProxy&, size_t numberOfFilledRows);

  size_t readTableT2(coral::ISessionProxy&);

  void writeTableT3(coral::ISessionProxy&, size_t numberOfFilledRows);

  size_t readTableT3(coral::ISessionProxy&);

protected:

  //test environment
  const TestEnv& m_env;

  coral::IConnectionService* m_connsvc;
  //default tables and views
  std::string T1;
  std::string T2;
  std::string T3;
  std::string V0;
  //session (OWNED)
  coral::ISessionProxy* m_session;

  coral::AttributeList* m_rowBuffer;
};

#endif
