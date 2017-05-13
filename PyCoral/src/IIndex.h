#ifndef PYCORAL_IINDEX_H
#define PYCORAL_IINDEX_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
