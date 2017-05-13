#ifndef PYCORAL_TIMESTAMP_H
#define PYCORAL_TIMESTAMP_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class TimeStamp;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::TimeStamp* object; // Object of coral::TimeStamp C++ class
      PyObject* parent;
    } TimeStamp;


    /// Returns the Python type
    PyTypeObject* TimeStamp_Type();

  }

}

#endif
