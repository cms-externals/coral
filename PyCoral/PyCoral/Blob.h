#ifndef PYCORAL_BLOB_H
#define PYCORAL_BLOB_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
    } Blob;


    /// Returns the Python type
    PyTypeObject* Blob_Type();

  }

}

#endif
