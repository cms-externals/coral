#include "AttributeUtils.h"

//CoralServerStubs includes
#include "Exceptions.h"

namespace coral { namespace CoralStubs {

  void
  copyAttributeListsEntries(AttributeList& dest, const AttributeList& source)
  {
    size_t size = source.size();
    if(dest.size() > 0)
      throw StreamBufferException("AttributeList is not empty", "RStreamBuffer::copyAttributeListsEntries");

    for(size_t i = 0; i < size; i++ )
    {
      const Attribute& attr_01 = source[i];
      const AttributeSpecification& spec01 = attr_01.specification();

      dest.extend(spec01.name(), spec01.type());
    }
  }

  void
  checkAttributeLists(AttributeList& dest, const AttributeList& source)
  {
    size_t size = source.size();
    if(size != dest.size())
      throw StreamBufferException("AttributeList missmatch, wrong size", "RStreamBuffer::copyAttributeLists");
    //check all attributes
    for(size_t i = 0; i < size; i++ ) {

      const Attribute& attr_01 = source[i];
      const Attribute& attr_02 = dest[i];

      //compare the both attributes
      const AttributeSpecification& spec01 = attr_01.specification();
      const AttributeSpecification& spec02 = attr_02.specification();

      if(spec01.name().compare(spec02.name()) != 0)
        throw StreamBufferException("AttributeList missmatch, name is not equal", "RStreamBuffer::copyAttributeLists");

      if(spec01.typeName().compare(spec02.typeName()) != 0)
        throw StreamBufferException("AttributeList missmatch, type-name is not equal", "RStreamBuffer::copyAttributeLists");

      if(spec01.type() != spec02.type())
        throw StreamBufferException("AttributeList missmatch, type is not equal", "RStreamBuffer::copyAttributeLists");

    }
  }

  void
  copyAttributeLists(AttributeList& dest, const AttributeList& source)
  {
    //perform a sanity check before copy operation
    checkAttributeLists(dest, source);
    //if everything is ok do the copy
    dest.fastCopyData( source );
  }

} }
