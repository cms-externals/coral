#ifndef PYCORAL_ATTRIBUTE_H
#define PYCORAL_ATTRIBUTE_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
