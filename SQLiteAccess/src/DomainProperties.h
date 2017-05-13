#ifndef SQLITEACCESS_DOMAINPROPERTIES_H
#define SQLITEACCESS_DOMAINPROPERTIES_H 1

#include <string>
#include "CoralBase/boost_thread_headers.h"

namespace coral
{
  class Service;
}

namespace coral
{
  namespace SQLiteAccess
  {

    /**
     * A simple class holding the global domain properties
     */

    class DomainProperties
    {
    public:
      /// Constructor
      explicit DomainProperties( coral::Service* service );

      /// Destructor
      ~DomainProperties();

      /// Returns the pointer to the service
      coral::Service* service() const;

      boost::mutex* mutex() const { return m_mutex; };


    private:
      /// The pointer to the service
      coral::Service*       m_service;

      boost::mutex* m_mutex;

    };

  }
}


inline coral::Service*
coral::SQLiteAccess::DomainProperties::service() const
{
  return m_service;
}

#endif
