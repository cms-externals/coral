#ifndef PYCORAL_IVIEW_H
#define PYCORAL_IVIEW_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
