#ifndef PYCORAL_IVIEW_H
#define PYCORAL_IVIEW_H

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
  class IView;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IView* object; // The underlying C++ type
      PyObject* parent;
    } IView;


    /// Returns the Python type
    PyTypeObject* IView_Type();

  }

}

#endif
