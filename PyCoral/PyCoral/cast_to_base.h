#ifndef CAST_TO_BASE_H
#define CAST_TO_BASE_H

#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif

PyObject* cast_to_base( PyObject* object,
                        PyTypeObject* baseType );

#endif
