#ifndef PYCORAL_IVIEWFACTORY_H
#define PYCORAL_IVIEWFACTORY_H

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
