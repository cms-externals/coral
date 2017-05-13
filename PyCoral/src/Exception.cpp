#include "Exception.h"

PyObject*
coral::PyCoral::Exception()
{
  static PyObject* coralException = PyErr_NewException( (char*) "coral.Exception",
                                                        0, 0 );
  return coralException;
}
