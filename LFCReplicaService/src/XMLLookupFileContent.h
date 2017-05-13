#ifndef LFCREPLICASERVICE_XMLLOOKUPFILECONTENT_H
#define LFCREPLICASERVICE_XMLLOOKUPFILECONTENT_H
#include <iostream>
#include "RelationalAccess/AccessMode.h"

namespace coral {

  namespace LFCReplicaService {

    class XMLLookupFileContent {

    public:

      explicit XMLLookupFileContent(std::ostream& out);

      bool openLcsEntry(const std::string& lcs);

      bool closeLcsEntry();

      bool addPcsEntry(const std::string& pcs,
                       const std::string& authent,
                       coral::AccessMode mode);

      void close();

    private:

      std::ostream& m_out;

      bool m_serviceEntryOpen;

      bool m_lcsEntryOpen;

      unsigned int m_ind;

    };

  }
}

#endif
