#ifndef PYCORAL_ITABLESCHEMAEDITOR_H
#define PYCORAL_ITABLESCHEMAEDITOR_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class ITableSchemaEditor;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITableSchemaEditor* object; // The underlying C++ type
      PyObject* parent;
    } ITableSchemaEditor;


    /// Returns the Python type
    PyTypeObject* ITableSchemaEditor_Type();

  }

}

#endif
