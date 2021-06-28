#ifndef PYCORAL_ATTRIBUTELISTITERATOR_H
#define PYCORAL_ATTRIBUTELISTITERATOR_H

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
