#ifndef PYCORAL_IUNIQUECONSTRAINT_H
#define PYCORAL_IUNIQUECONSTRAINT_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
