#include "cast_to_base.h"

typedef union {
  PyObject* object;
  char* address;
  PyObject** base;
} U_A;

#include <set>

static PyObject*
_cast_to_base( PyObject* object,
               PyTypeObject* baseType,
               std::set<PyTypeObject*>& classesTried )
{
  // Sanity checks
  if ( ! object || ! baseType || ! object->ob_type ||
       classesTried.find( object->ob_type ) != classesTried.end() ) return 0;

  // This is the correct type!
  if ( object->ob_type == baseType ) {
    return object;
  }

  // Check if the type has the proper base class
  if ( ! PyType_IsSubtype( object->ob_type, baseType ) ) return 0;

  classesTried.insert( object->ob_type );

  // Get the tuple of bases
  PyObject* bases = object->ob_type->tp_bases;

  // No other base classes
  if ( ! bases ) return 0;

  U_A baseAddress;
  baseAddress.object = object;
  baseAddress.address += sizeof( PyObject );

  int numberOfBases = PyTuple_GET_SIZE( bases );

  for ( int i = 0; i < numberOfBases; ++i ) {
    PyObject* currentBase = _cast_to_base( *( baseAddress.base ), baseType, classesTried );
    if ( currentBase ) return currentBase;
    baseAddress.address += sizeof(PyObject*);
  }

  return 0;
}


PyObject*
cast_to_base( PyObject* object,
              PyTypeObject* baseType )
{
  std::set<PyTypeObject*> classesTried;
  return _cast_to_base( object, baseType, classesTried );
}
