#ifndef PYCORAL_ITYPECONVERTER_H
#define PYCORAL_ITYPECONVERTER_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class ITypeConverter;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITypeConverter* object; // The underlying C++ type
      PyObject* parent;
    } ITypeConverter;


    /// Returns the Python type
    PyTypeObject* ITypeConverter_Type();

  }

}

#endif
