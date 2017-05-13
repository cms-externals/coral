#ifndef PYCORAL_ISCHEMA_H
#define PYCORAL_ISCHEMA_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class ISchema;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ISchema* object; // The underlying C++ type
      PyObject* parent;
    } ISchema;


    /// Returns the Python type
    PyTypeObject* ISchema_Type();

  }

}

#endif
