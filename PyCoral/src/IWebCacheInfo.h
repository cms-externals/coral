#ifndef PYCORAL_IWEBCACHEINFO_H
#define PYCORAL_IWEBCACHEINFO_H

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
  class IWebCacheInfo;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IWebCacheInfo* object; // The underlying C++ type
      PyObject* parent;
    } IWebCacheInfo;


    /// Returns the Python type
    PyTypeObject* IWebCacheInfo_Type();

  }

}

#endif
