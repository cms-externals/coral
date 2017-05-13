#ifndef PYCORAL_IWEBCACHECONTROL_H
#define PYCORAL_IWEBCACHECONTROL_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IWebCacheControl;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IWebCacheControl* object; // The underlying C++ type
      PyObject* parent;
    } IWebCacheControl;


    /// Returns the Python type
    PyTypeObject* IWebCacheControl_Type();

  }

}

#endif
