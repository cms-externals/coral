#ifndef PYCORAL_IBULKOPERATION_H
#define PYCORAL_IBULKOPERATION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
