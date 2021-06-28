#ifndef PYCORAL_ICONNECTIONSERVICECONFIGURATION_H
#define PYCORAL_ICONNECTIONSERVICECONFIGURATION_H

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

  class IConnectionServiceConfiguration;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IConnectionServiceConfiguration* object; // The underlying C++ type
      PyObject* parent; // A reference to the parent object
    } IConnectionServiceConfiguration;

    /// Returns the Python type
    PyTypeObject* IConnectionServiceConfiguration_Type();

  }

}

#endif
