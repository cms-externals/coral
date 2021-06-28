#ifndef PYCORAL_BLOB_H
#define PYCORAL_BLOB_H

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
  class Blob;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::Blob* object; // The underlying C++ type
      PyObject* parent;
      long currentPosition;
      #if PY_MAJOR_VERSION >= 3
      int ob_exports;      /* How many buffer exports */
      #endif
    } Blob;


    /// Returns the Python type
    PyTypeObject* Blob_Type();

  }

}

#endif
