#ifndef PYCORAL_CONNECTIONSERVICE_H
#define PYCORAL_CONNECTIONSERVICE_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

//#include "Context.h"

namespace coral {

  class IConnectionService;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::IConnectionService* object; // The underlying C++ type
      //      Context* context;
    } ConnectionService;

    /// Returns the Python type
    PyTypeObject* ConnectionService_Type();

  }

}

#endif
