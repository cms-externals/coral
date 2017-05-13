#ifndef PYCORAL_DATE_H
#define PYCORAL_DATE_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
