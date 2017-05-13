#ifndef PYCORAL_EXCEPTION_H
#define PYCORAL_EXCEPTION_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

namespace coral {

  namespace PyCoral {

    /// Returns the exception object of this module
    PyObject* Exception();

  }

}

#endif
