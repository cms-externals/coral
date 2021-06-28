#ifndef PYCORAL_ITABLESCHEMAEDITOR_H
#define PYCORAL_ITABLESCHEMAEDITOR_H

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
