#ifndef PYCORAL_ISESSIONPROXY_H
#define PYCORAL_ISESSIONPROXY_H

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
  class ISessionProxy;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ISessionProxy* object; // The underlying C++ type
      PyObject* parent;
    } ISessionProxy;


    /// Returns the Python type
    PyTypeObject* ISessionProxy_Type();

  }

}

#endif
