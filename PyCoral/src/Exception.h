#ifndef PYCORAL_EXCEPTION_H
#define PYCORAL_EXCEPTION_H

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

  namespace PyCoral {

    /// Returns the exception object of this module
    PyObject* Exception();

  }

}

#endif
