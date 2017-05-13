#ifndef ORACLEACCESS_ORACLEDOMAINPROPERTYNAMES_H
#define ORACLEACCESS_ORACLEDOMAINPROPERTYNAMES_H

#include <string>

namespace coral {

  namespace OracleAccess {

    /// A class holding the names of the properties of the Domain component

    class DomainPropertyNames {
    public:
      static std::string tableSpaceForTables();
      static std::string tableSpaceForIndices();
      static std::string tableSpaceForLobs();
      static std::string lobChunkSize();
    };

  }

}

#endif
