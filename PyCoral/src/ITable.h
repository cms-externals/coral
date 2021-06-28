#ifndef PYCORAL_ITABLE_H
#define PYCORAL_ITABLE_H

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
  class ITable;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITable* object; // The underlying C++ type
      PyObject* parent;
    } ITable;


    /// Returns the Python type
    PyTypeObject* ITable_Type();

  }

}

#endif
