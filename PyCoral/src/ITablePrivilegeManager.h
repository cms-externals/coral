#ifndef PYCORAL_ITABLEPRIVILEGEMANAGER_H
#define PYCORAL_ITABLEPRIVILEGEMANAGER_H

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
  class ITablePrivilegeManager;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::ITablePrivilegeManager* object; // The underlying C++ type
      PyObject* parent;
    } ITablePrivilegeManager;


    /// Returns the Python type
    PyTypeObject* ITablePrivilegeManager_Type();

  }

}

#endif
