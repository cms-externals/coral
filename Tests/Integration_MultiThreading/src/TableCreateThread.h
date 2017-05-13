#ifndef TEST_MTHREAD_TABLECREATETHREAD_H
#define TEST_MTHREAD_TABLECREATETHREAD_H 1

#include <string>

namespace coral {
  class ISchema;
}

class Reporter;

class TableCreateThread
{

public:
  // Constructor
  TableCreateThread( coral::ISchema& schema,
                     int tableNumber,
                     const std::string& table )
    : m_schema( schema )
    , m_tableNumber( tableNumber )
    , m_table( table )
  {}

  // Destructor
  ~TableCreateThread() {}

  // The thread body
  void operator()();

private:

  coral::ISchema& m_schema;

  int m_tableNumber;

  std::string m_table;

};

#endif
