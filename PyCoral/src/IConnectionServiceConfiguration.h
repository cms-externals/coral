#ifndef PYCORAL_ICONNECTIONSERVICECONFIGURATION_H
#define PYCORAL_ICONNECTIONSERVICECONFIGURATION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
