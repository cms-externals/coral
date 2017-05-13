#include "CoralKernel/Property.h"
#include "PropertyManager.h"

namespace coral {

  PropertyManager::PropertyManager()
  {
    _setDefaultProperties();
  }

  PropertyManager::~PropertyManager()
  {
    // EMPTY
  }

  IProperty* PropertyManager::property(const std::string& key)
  {
    MONITOR_START_CRITICAL
      M_Container::iterator loc = m_cont.find(key);
    return loc == m_cont.end() ? NULL : loc->second.get();
    MONITOR_END_CRITICAL
      }

  void PropertyManager::_setDefaultProperties()
  {
    _addProperty("CredentialDatabase", "sqlite_file:coral_credentials.db");
    _addProperty("EncryptionKey","");
    _addProperty("AuthenticationFile", "authentication.xml");
    _addProperty("DBLookupFile", "dblookup.xml");
    _addProperty("Server_Hostname", "localhost");
    _addProperty("Server_Port", "8080");
  }

  void PropertyManager::_addProperty(const char* key, const char* value)
  {
    boost::shared_ptr<IProperty> prop(new Property);
    prop->set(value);
    m_cont[key] = prop;
  }

} // namespace coral
