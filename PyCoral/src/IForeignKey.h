#ifndef PYCORAL_IFOREIGNKEY_H
#define PYCORAL_IFOREIGNKEY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IForeignKey;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IForeignKey* object; // The underlying C++ type
      PyObject* parent;
    } IForeignKey;


    /// Returns the Python type
    PyTypeObject* IForeignKey_Type();

  }

}

#endif
