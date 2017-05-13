#ifndef INCLUDE_TESTAPP_H
#define INCLUDE_TESTAPP_H

#include "RelationalAccess/IConnectionService.h"
#ifdef CORAL_SEALED
#include "SealKernel/Context.h"
#endif
#include <string>

class TestApp
{
public:
  static const std::string OVALTAG;

  // Constructor
  TestApp();

  // Destructor
  ~TestApp();

  // run method
  void run();

private:
  // Loads the basic services
  void loadServices();

#ifdef CORAL_SEALED
  // Returns the connection service handle
  seal::IHandle<coral::IConnectionService> connectionService();

  //
  void setupDataSource( coral::ISessionProxy& session);

private:
  // The application context
  seal::Handle<seal::Context> m_context;
#endif
};

#endif
