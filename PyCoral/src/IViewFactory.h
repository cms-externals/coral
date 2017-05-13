#ifndef PYCORAL_IVIEWFACTORY_H
#define PYCORAL_IVIEWFACTORY_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  // forward declaration of the underlying C++ type
  class IViewFactory;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      PyObject* base1;
      PyObject* parent;
      coral::IViewFactory* object; // The underlying C++ type
    } IViewFactory;


    /// Returns the Python type
    PyTypeObject* IViewFactory_Type();

  }

}

#endif
