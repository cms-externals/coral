#ifndef PROPERTYMANAGER_H_
#define PROPERTYMANAGER_H_

#include <map>
#include <boost/shared_ptr.hpp>

#include "CoralKernel/MonitorObject.h"
#include "CoralKernel/IPropertyManager.h"

namespace coral {
  /**
   * Class PropertyManager
   * Implementation of the IPropertyManager class. PropertyManager objects are
   * thread-safe (MonitorObject-s). The object stores a fixed property set,
   * the set cannot be modified in runtime (the property values can be modified).
   */
  class PropertyManager : public IPropertyManager, public MonitorObject {
  public:
    PropertyManager();
    virtual ~PropertyManager();
    virtual IProperty* property(const std::string&);

  private:
    void _setDefaultProperties();
    void _addProperty(const char* key, const char* value);

    typedef std::map<std::string, boost::shared_ptr<IProperty> > M_Container;
    M_Container m_cont;
  };

} // namespace coral

#endif /*PROPERTYMANAGER_H_*/
