#ifndef PYCORAL_ITRANSACTION_H
#define PYCORAL_ITRANSACTION_H

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
  class ITransaction;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITransaction* object; // The underlying C++ type
      PyObject* parent;
    } ITransaction;


    /// Returns the Python type
    PyTypeObject* ITransaction_Type();

  }

}

#endif
