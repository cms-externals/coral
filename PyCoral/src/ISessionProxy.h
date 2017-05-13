#ifndef PYCORAL_ISESSIONPROXY_H
#define PYCORAL_ISESSIONPROXY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
