#ifndef PYCORAL_ICOLUMN_H
#define PYCORAL_ICOLUMN_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IColumn;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IColumn* object; // The underlying C++ type
      PyObject* parent;
    } IColumn;


    /// Returns the Python type
    PyTypeObject* IColumn_Type();

  }

}

#endif
