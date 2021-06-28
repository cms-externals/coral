#ifndef PYCORAL_ISCHEMA_H
#define PYCORAL_ISCHEMA_H

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
