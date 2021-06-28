#ifndef PYCORAL_MESSAGESTREAM_H
#define PYCORAL_MESSAGESTREAM_H

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
  class MessageStream;

  namespace PyCoral {

    typedef struct {
      PyObject_HEAD
      coral::MessageStream* object; // The underlying C++ type
      PyObject* parent;
    } MessageStream;

    /// Returns the Python type
    PyTypeObject* MessageStream_Type();

  }

}

#endif
