#ifndef PYCORAL_IQUERY_H
#define PYCORAL_IQUERY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IQuery;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      PyObject* base1;
      PyObject* parent;
      coral::IQuery* object; // The underlying C++ type
    } IQuery;


    /// Returns the Python type
    PyTypeObject* IQuery_Type();

  }

}

#endif
