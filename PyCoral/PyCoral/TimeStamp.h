#ifndef PYCORAL_TIMESTAMP_H
#define PYCORAL_TIMESTAMP_H

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
