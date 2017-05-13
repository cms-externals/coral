#ifndef PYCORAL_TABLEDESCRIPTION_H
#define PYCORAL_TABLEDESCRIPTION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class TableDescription;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      PyObject* base1;
      PyObject* base2;
      coral::TableDescription* object; // The underlying C++ type
    } TableDescription;


    /// Returns the Python type
    PyTypeObject* TableDescription_Type();

  }

}

#endif
