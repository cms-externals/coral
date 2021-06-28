#ifndef CAST_TO_BASE_H
#define CAST_TO_BASE_H

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

PyObject* cast_to_base( PyObject* object,
                        PyTypeObject* baseType );

#endif
