#ifndef PYCORAL_ITABLEDATAEDITOR_H
#define PYCORAL_ITABLEDATAEDITOR_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
