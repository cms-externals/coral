#ifndef CORAL_CORALSTUBS_ATTRUTILS_H
#define CORAL_CORALSTUBS_ATTRUTILS_H

//Coral includes
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

namespace coral {

  namespace CoralStubs {

    void copyAttributeListsEntries(AttributeList& dest, const AttributeList& source);

    void checkAttributeLists(AttributeList& dest, const AttributeList& source);

    void copyAttributeLists(AttributeList& dest, const AttributeList& source);

    void getMagicNumber(const AttributeList& source);

  }

}

#endif
