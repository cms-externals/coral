#ifndef PYCORAL_IWEBCACHEINFO_H
#define PYCORAL_IWEBCACHEINFO_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
