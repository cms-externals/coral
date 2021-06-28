#ifndef PYCORAL_IUNIQUECONSTRAINT_H
#define PYCORAL_IUNIQUECONSTRAINT_H

#ifdef _DEBUG
#undef _DEBUG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#include "Python.h"
#pragma clang diagnostic pop
#define _DEBUG
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#include "Python.h"
#pragma clang diagnostic pop
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IUniqueConstraint;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IUniqueConstraint* object; // The underlying C++ type
      PyObject* parent;
    } IUniqueConstraint;


    /// Returns the Python type
    PyTypeObject* IUniqueConstraint_Type();

  }

}

#endif
