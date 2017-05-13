#ifndef PYCORAL_ITABLE_H
#define PYCORAL_ITABLE_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
