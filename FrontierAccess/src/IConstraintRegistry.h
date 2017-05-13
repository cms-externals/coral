#ifndef CONSTRAINT_REGISTRY_H
#define CONSTRAINT_REGISTRY_H

namespace coral {

  namespace FrontierAccess {

    class IConstraintRegistry {
    public:
      virtual ~IConstraintRegistry() {};

      virtual void refreshConstraints() const = 0;
    };

  }

}

#endif
