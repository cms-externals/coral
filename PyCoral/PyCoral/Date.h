#ifndef PYCORAL_DATE_H
#define PYCORAL_DATE_H

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
  class Date;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::Date* object; // The underlying C++ type
      PyObject* parent;
    } Date;


    /// Returns the Python type
    PyTypeObject* Date_Type();

  }

}

#endif
