#ifndef PYCORAL_IBULKOPERATION_H
#define PYCORAL_IBULKOPERATION_H

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
  class IBulkOperation;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IBulkOperation* object; // The underlying C++ type
      PyObject* parent;
    } IBulkOperation;


    /// Returns the Python type
    PyTypeObject* IBulkOperation_Type();

  }

}

#endif
