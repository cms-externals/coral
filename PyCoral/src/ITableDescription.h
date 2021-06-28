#ifndef PYCORAL_ITABLEDESCRIPTION_H
#define PYCORAL_ITABLEDESCRIPTION_H

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
