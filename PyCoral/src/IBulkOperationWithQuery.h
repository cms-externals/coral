#ifndef PYCORAL_IBULKOPERATIONWITHQUERY_H
#define PYCORAL_IBULKOPERATIONWITHQUERY_H

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
  class IBulkOperationWithQuery;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      PyObject* base1;
      coral::IBulkOperationWithQuery* object; // The underlying C++ type
      PyObject* parent;
    } IBulkOperationWithQuery;


    /// Returns the Python type
    PyTypeObject* IBulkOperationWithQuery_Type();

  }

}

#endif
