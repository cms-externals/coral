#ifndef PYCORAL_ATTRIBUTELIST_H
#define PYCORAL_ATTRIBUTELIST_H

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
  class AttributeList;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::AttributeList* object; // The underlying C++ type
      PyObject* parent;
    } AttributeList;


    /// Returns the Python type
    PyTypeObject* AttributeList_Type();

  }

}

#endif
