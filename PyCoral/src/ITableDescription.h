#ifndef PYCORAL_ITABLEDESCRIPTION_H
#define PYCORAL_ITABLEDESCRIPTION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class ITableDescription;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITableDescription* object; // The underlying C++ type
      PyObject* parent;
    } ITableDescription;


    /// Returns the Python type
    PyTypeObject* ITableDescription_Type();

  }

}

#endif
