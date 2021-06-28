#ifndef PYCORAL_IQUERYDEFINITION_H
#define PYCORAL_IQUERYDEFINITION_H

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
  class IQueryDefinition;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      PyObject* parent;
      coral::IQueryDefinition* object; // The underlying C++ type
    } IQueryDefinition;


    /// Returns the Python type
    PyTypeObject* IQueryDefinition_Type();

  }

}

#endif
