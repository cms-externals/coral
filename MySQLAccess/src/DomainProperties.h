// $Id: DomainProperties.h,v 1.6 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_MYSQLDOMAIN_PROPERTIES_H
#define MYSQLACCESS_MYSQLDOMAIN_PROPERTIES_H 1

#include <string>
#include "CoralKernel/IHandle.h"

namespace coral
{
  class Service;
  class ITypeConverter;

  namespace MySQLAccess
  {
    class Domain;
    class TypeConverter;

    /**
     * A simple class holding the global domain properties
     */

    class DomainProperties
    {
    public:
      /// Constructor
      explicit DomainProperties( coral::MySQLAccess::Domain* service );

      /// Destructor
      ~DomainProperties();

      /// Returns the pointer to the service
      coral::Service* service() const;

      /// The type converter
      coral::ITypeConverter& typeConverter() const;

    private:
      /// The pointer to the service
      coral::MySQLAccess::Domain*         m_service;
      /// The type converter
      coral::MySQLAccess::TypeConverter*  m_typeConverter;
    };
  }
}

#endif // MYSQLACCESS_MYSQLDOMAIN_PROPERTIES_H
