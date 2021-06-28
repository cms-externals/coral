#ifndef PYCORAL_IMONITORINGREPORTER_H
#define PYCORAL_IMONITORINGREPORTER_H

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
  class IMonitoringReporter;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IMonitoringReporter* object; // The underlying C++ type
      PyObject* parent;
    } IMonitoringReporter;


    /// Returns the Python type
    PyTypeObject* IMonitoringReporter_Type();

  }

}

#endif
