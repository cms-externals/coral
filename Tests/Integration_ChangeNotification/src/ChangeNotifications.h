#ifndef TEST_CHANGENOTIFICATION_H
#define TEST_CHANGENOTIFICATION_H 1

#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/../src/IChangeNotification.h" // temporary moved to src
#include "TestEnv/Testing.h"

#ifdef CORAL240CN

class Notification : public coral::IChangeNotification {
public:

  void event();

};

#endif

class ChangeNotifications : public Testing
{
public:

  ChangeNotifications( const TestEnv& env );

  virtual ~ChangeNotifications();

  void setup();

  void test01();

private:

#ifdef CORAL240CN
  Notification m_notification;
#endif

};

#endif
