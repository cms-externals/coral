#ifndef PYCORAL_ICURSORITERATOR_H
#define PYCORAL_ICURSORITERATOR_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
