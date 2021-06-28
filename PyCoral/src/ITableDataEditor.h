#ifndef PYCORAL_ITABLEDATAEDITOR_H
#define PYCORAL_ITABLEDATAEDITOR_H

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
  class ITableDataEditor;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITableDataEditor* object; // The underlying C++ type
      PyObject* parent;
    } ITableDataEditor;


    /// Returns the Python type
    PyTypeObject* ITableDataEditor_Type();

  }

}

#endif
