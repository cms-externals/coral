#include "AttributeListIterator.h"
#include "PyCoral/AttributeList.h"
#include "Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "Exception.h"

#ifdef _DEBUG
#undef _DEBUG
#include "structmember.h"
#define _DEBUG
#else
#include "structmember.h"
#endif

// Forward declaration of the methods
static int AttributeListIterator_init( PyObject* self, PyObject*, PyObject* );
static void AttributeListIterator_dealloc( PyObject* self );
static PyObject* AttributeListIterator_next( PyObject* self );
static PyObject* AttributeListIterator_iter( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::AttributeListIterator_Type()
{
  static PyMethodDef AttributeListIterator_Methods[] = {
    {0, 0, 0, 0}
  };

  static PyMemberDef AttributeListIterator_Members[]= {
    {0,0,0,0,0}
  };

  static PyTypeObject AttributeListIterator_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char *)"coral.AttributeListIterator", /*tp_name*/
    sizeof(coral::PyCoral::AttributeListIterator), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    AttributeListIterator_dealloc, /*tp_dealloc*/
    0, /*tp_print*/
    0, /*tp_getattr*/
    0, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr*/
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash*/
    0, /*tp_call*/
    0, /*tp_str*/
    PyObject_GenericGetAttr, /*tp_getattro*/
    PyObject_GenericSetAttr, /*tp_setattro*/
    0, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    0, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    AttributeListIterator_iter, /*tp_iter*/
    AttributeListIterator_next, /*tp_iternext*/
    AttributeListIterator_Methods, /*tp_methods*/
    AttributeListIterator_Members, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    AttributeListIterator_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    AttributeListIterator_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &AttributeListIterator_Type;
}


// Implementation of the methods
int
AttributeListIterator_init( PyObject* self, PyObject*  args, PyObject* /* kwds */ )
{
  coral::PyCoral::AttributeListIterator* py_this = (coral::PyCoral::AttributeListIterator*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );

    return -1;
  }
  py_this->parent = 0;
  py_this->currentPosition = 0;

  if ( !PyArg_Parse( args, (char*)"O", &(py_this->parent) ) ) return -1;

  if ( py_this->parent ) Py_INCREF( py_this->parent );

  return 0;
}


void
AttributeListIterator_dealloc( PyObject* self )
{
  coral::PyCoral::AttributeListIterator* py_this = (coral::PyCoral::AttributeListIterator*) self;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }

  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
AttributeListIterator_next( PyObject* self )
{
  coral::PyCoral::AttributeListIterator* py_this = (coral::PyCoral::AttributeListIterator*) self;
  if( ! py_this->parent->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->currentPosition >= (long)py_this->parent->object->size() ) return 0;
    const coral::AttributeList& theList = *(py_this->parent->object);
    const coral::Attribute& theAttribute = theList[ (unsigned int)py_this->currentPosition ];
    coral::Attribute* element = const_cast< coral::Attribute* >( &theAttribute );
    coral::PyCoral::Attribute* ob = PyObject_New( coral::PyCoral::Attribute, coral::PyCoral::Attribute_Type() );

    ( py_this->currentPosition )++;

    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating Attribute object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( element,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object);
    bool ok = (ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Initialising Attribute object." );
      PyObject_Del(ob);
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in iteration over AttributeList" );
    return 0;
  }
}


PyObject*
AttributeListIterator_iter( PyObject* self )
{
  coral::PyCoral::AttributeListIterator* py_this = (coral::PyCoral::AttributeListIterator*) self;
  if( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid Object" );
    return 0;
  }
  Py_INCREF(self);
  return self;
}
