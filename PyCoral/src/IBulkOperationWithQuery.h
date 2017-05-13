#ifndef PYCORAL_IBULKOPERATIONWITHQUERY_H
#define PYCORAL_IBULKOPERATIONWITHQUERY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
