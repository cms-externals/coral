#ifndef PYCORAL_ICURSOR_H
#define PYCORAL_ICURSOR_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class ICursor;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ICursor* object; // The underlying C++ type
      PyObject* parent;
    } ICursor;


    /// Returns the Python type
    PyTypeObject* ICursor_Type();

  }

}

#endif
