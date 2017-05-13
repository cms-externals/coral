#ifndef PYCORAL_ITABLEPRIVILEGEMANAGER_H
#define PYCORAL_ITABLEPRIVILEGEMANAGER_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
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
