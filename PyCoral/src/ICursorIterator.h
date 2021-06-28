#ifndef PYCORAL_ICURSORITERATOR_H
#define PYCORAL_ICURSORITERATOR_H

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
#include "ICursor.h"

namespace coral {
  class ICursor;

  namespace PyCoral {
    typedef struct {
      PyObject_HEAD
      ICursor* parent;
      long currentPosition;
    } ICursorIterator;

    // Initializes the type
    PyTypeObject* ICursorIterator_Type();
  }

}

#endif
