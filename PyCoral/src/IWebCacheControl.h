#ifndef PYCORAL_IWEBCACHECONTROL_H
#define PYCORAL_IWEBCACHECONTROL_H

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
