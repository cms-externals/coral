#ifndef PYCORAL_ATTRIBUTE_H
#define PYCORAL_ATTRIBUTE_H

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

  // forward declaration
  class Attribute;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::Attribute* object; // Object of Attribute class
      PyObject* parent;
    } Attribute;


    /// Returns the Python type
    PyTypeObject* Attribute_Type();

  }

}

#endif
