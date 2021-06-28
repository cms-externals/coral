#ifndef PYCORAL_IFOREIGNKEY_H
#define PYCORAL_IFOREIGNKEY_H

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
