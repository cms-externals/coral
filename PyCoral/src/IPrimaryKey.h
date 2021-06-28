#ifndef PYCORAL_IPRIMARYKEY_H
#define PYCORAL_IPRIMARYKEY_H

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
  class IPrimaryKey;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IPrimaryKey* object; // The underlying C++ type
      PyObject* parent;
    } IPrimaryKey;


    /// Returns the Python type
    PyTypeObject* IPrimaryKey_Type();

  }

}

#endif
