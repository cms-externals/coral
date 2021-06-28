#ifndef PYCORAL_ICOLUMN_H
#define PYCORAL_ICOLUMN_H

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
  class IColumn;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IColumn* object; // The underlying C++ type
      PyObject* parent;
    } IColumn;


    /// Returns the Python type
    PyTypeObject* IColumn_Type();

  }

}

#endif
