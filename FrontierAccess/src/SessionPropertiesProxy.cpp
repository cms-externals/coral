#include "SessionPropertiesProxy.h"
#include "RelationalAccess/ISession.h"

coral::ISchema&
coral::FrontierAccess::SessionPropertiesProxy::schema() const
{
  return const_cast< coral::FrontierAccess::SessionProperties&>( m_properties ).session().schema( m_schemaName );
}
