#ifndef PYCORAL_IINDEX_H
#define PYCORAL_IINDEX_H

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
  class IIndex;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IIndex* object; // The underlying C++ type
      PyObject* parent;
    } IIndex;


    /// Returns the Python type
    PyTypeObject* IIndex_Type();

  }

}

#endif
