#ifndef PYCORAL_IOPERATIONWITHQUERY_H
#define PYCORAL_IOPERATIONWITHQUERY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IOperationWithQuery;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IOperationWithQuery* object; // The underlying C++ type
      PyObject* parent;
    } IOperationWithQuery;


    /// Returns the Python type
    PyTypeObject* IOperationWithQuery_Type();

  }

}

#endif
