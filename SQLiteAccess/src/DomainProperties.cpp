#include "CoralBase/boost_thread_headers.h"
#include "DomainProperties.h"

namespace coral
{
  namespace SQLiteAccess
  {

    DomainProperties::DomainProperties( coral::Service* service )
      : m_service( service )
      , m_mutex( new boost::mutex() )
    {
    }


    DomainProperties::~DomainProperties()
    {
      delete m_mutex;
    }

  }
}
