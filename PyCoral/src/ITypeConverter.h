#ifndef PYCORAL_ITYPECONVERTER_H
#define PYCORAL_ITYPECONVERTER_H

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
