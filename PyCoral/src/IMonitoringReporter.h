#ifndef PYCORAL_IMONITORINGREPORTER_H
#define PYCORAL_IMONITORINGREPORTER_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
