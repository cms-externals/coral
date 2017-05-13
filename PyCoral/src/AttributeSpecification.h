#ifndef PYCORAL_ATTRIBUTESPECIFICATION_H
#define PYCORAL_ATTRIBUTESPECIFICATION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
