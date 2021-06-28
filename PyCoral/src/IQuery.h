#ifndef PYCORAL_IQUERY_H
#define PYCORAL_IQUERY_H

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
