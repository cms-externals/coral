#ifndef PYCORAL_ISESSIONPROPERTIES_H
#define PYCORAL_ISESSIONPROPERTIES_H

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
  class ISessionProperties;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ISessionProperties* object; // The underlying C++ type
      PyObject* parent;
    } ISessionProperties;


    /// Returns the Python type
    PyTypeObject* ISessionProperties_Type();

  }

}

#endif
