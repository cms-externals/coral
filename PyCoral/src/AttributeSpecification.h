#ifndef PYCORAL_ATTRIBUTESPECIFICATION_H
#define PYCORAL_ATTRIBUTESPECIFICATION_H

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
  class AttributeSpecification;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::AttributeSpecification* object; // The underlying C++ type
      PyObject* parent;
    } AttributeSpecification;


    /// Returns the Python type
    PyTypeObject* AttributeSpecification_Type();

  }

}

#endif
