#ifndef CORALBASE_ATTRIBUTEDATAFACTORY_H
#define CORALBASE_ATTRIBUTEDATAFACTORY_H

#include <typeinfo>

namespace coral {

  class AttributeData;

  class AttributeDataFactory {
  public:
    AttributeData* create( const std::type_info& type ) const;
    static const AttributeDataFactory& factory();
  protected:
    AttributeDataFactory() {}
  };
}

#endif
