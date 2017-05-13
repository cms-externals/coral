#ifndef IPROPERTYMANAGER_H_
#define IPROPERTYMANAGER_H_

#include <string>
#include "IProperty.h"

// Temporary fix for bug #63198 (struct was first seen as class).
// The API is internally inconsistent: Context.h forward declares a class:
// eventually struct IPropertyManager will become a class.
#ifdef WIN32
#pragma warning ( disable : 4099 )
#endif

namespace coral
{

  /**
   * @class IPropertyManager
   *
   * Interface for an object managing a set of properties. The object must
   * store a fixed property set, properties cannot be added/deleted by this
   * interface. The properties are identified by strings (names).
   *
   * @author Zsolt Molnar
   * @date   2008-05-21
   */
  struct IPropertyManager
  {

  public:

    virtual ~IPropertyManager() {}

    /**
     * Return the property identified by the given string or NULL if the
     * property does not exist.
     */
    virtual IProperty* property(const std::string&) = 0;

  };

}
#endif /*IPROPERTYMANAGER_H_*/
