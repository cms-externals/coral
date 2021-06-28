#ifndef PYCORAL_ICURSOR_H
#define PYCORAL_ICURSOR_H

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
