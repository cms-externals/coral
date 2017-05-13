#ifndef FRONTIER_ACCESS_FRONTIERDOMAINPROPERTYNAMES_H
#define FRONTIER_ACCESS_FRONTIERDOMAINPROPERTYNAMES_H

#include <string>

namespace coral {

  namespace FrontierAccess {

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
