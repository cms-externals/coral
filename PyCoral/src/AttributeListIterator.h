#ifndef PYCORAL_ATTRIBUTELISTITERATOR_H
#define PYCORAL_ATTRIBUTELISTITERATOR_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif
#include "PyCoral/AttributeList.h"

namespace coral {
  class AttributeList;

  namespace PyCoral {
    typedef struct {
      PyObject_HEAD
      AttributeList* parent;
      long currentPosition;
    } AttributeListIterator;

    // Initializes the type
    PyTypeObject* AttributeListIterator_Type();
  }

}

#endif
