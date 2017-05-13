#ifndef RELATIONALACCESS_ICHANGENOTIFICATION_H
#define RELATIONALACCESS_ICHANGENOTIFICATION_H

#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

namespace coral
{

#ifdef CORAL240CN
  /**
   * Class IChangeNotification
   *
   * Abstract class with one method to be called if
   * a notification was received
   *
   * TODO: add more members to identify the table, etc...
   */
  class IChangeNotification
  {

  public:

    /**
     * This method is call, when a notification was send
     *
     * TODO: add more parameters for analysis
     */
    virtual void event() = 0;

  };
#endif

}
#endif
