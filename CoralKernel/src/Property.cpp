#include "CoralKernel/Property.h"

namespace coral {

  Property::Property()
    : m_actualID(0)
  {
    // EMPTY
  }

  Property::~Property()
  {
    // EMPTY
  }

  bool Property::set(const std::string& value)
  {
    MONITOR_START_CRITICAL
      m_value = value;
    for(M_Callbacks::iterator i = m_callbacks.begin(); i != m_callbacks.end(); ++i) i->second(m_value);
    return true;
    MONITOR_END_CRITICAL
      }

  const std::string& Property::get() const
  {
    MONITOR_START_CRITICAL
      return m_value;
    MONITOR_END_CRITICAL
      }

  Property::CallbackID Property::registerCallback(Property::Callback& cb)
  {
    MONITOR_START_CRITICAL
      m_callbacks[m_actualID] = cb;
    return m_actualID++;
    MONITOR_END_CRITICAL
      }

  bool Property::unregisterCallback(Property::CallbackID& id)
  {
    MONITOR_START_CRITICAL
      return m_callbacks.erase(id) > 0;
    MONITOR_END_CRITICAL
      }

} // namespace coral
