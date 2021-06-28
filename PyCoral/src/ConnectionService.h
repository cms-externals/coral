#ifndef PYCORAL_CONNECTIONSERVICE_H
#define PYCORAL_CONNECTIONSERVICE_H 1

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

namespace coral
{

  //class IConnectionService; // private destructor, cannot be deleted
  class ConnectionService; // fix bug #100573

  namespace PyCoral
  {

    typedef struct
    {
      PyObject_HEAD
      coral::ConnectionService* object; // The underlying C++ type
    } ConnectionService;

    /// Returns the Python type
    PyTypeObject* ConnectionService_Type();

  }

}

#endif
