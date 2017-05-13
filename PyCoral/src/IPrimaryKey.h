#ifndef PYCORAL_IPRIMARYKEY_H
#define PYCORAL_IPRIMARYKEY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
