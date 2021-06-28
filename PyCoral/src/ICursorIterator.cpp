#include "ICursorIterator.h"
#include "ICursor.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/ICursor.h"
#include "CoralBase/AttributeList.h"
#include "Exception.h"

#ifdef _DEBUG
#undef _DEBUG
#include "structmember.h"
#define _DEBUG
#else
#include "structmember.h"
#endif


// Forward declaration of the methods
static int ICursorIterator_init( PyObject* self, PyObject* args, PyObject* );
static void ICursorIterator_dealloc( PyObject* self );
static PyObject* ICursorIterator_next( PyObject* self );
static PyObject* ICursorIterator_iter( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::ICursorIterator_Type()
{
  static PyMethodDef ICursorIterator_Methods[] = {
    {0, 0, 0, 0}
  };

  static PyMemberDef ICursorIterator_Members[]= {
    {0, 0,0,0,0}
  };

  static PyTypeObject ICursorIterator_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char *)"coral.ICursorIterator", // tp_name
    sizeof(coral::PyCoral::ICursorIterator), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ICursorIterator_dealloc, // tp_dealloc
    0, // tp_print
    0, // tp_getattr
    0, // tp_setattr
    0, // tp_compare
    0, // tp_repr
    0, // tp_as_number
    0, // tp_as_sequence
    0, // tp_as_mapping
    0, // tp_hash
    0, // tp_call
    0, // tp_str
    PyObject_GenericGetAttr, // tp_getattro
    PyObject_GenericSetAttr, // tp_setattro
    0, // tp_as_buffer
    Py_TPFLAGS_DEFAULT, // tp_flags
    0, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    ICursorIterator_iter, // tp_iter
    ICursorIterator_next, // tp_iternext
    ICursorIterator_Methods, // tp_methods
    ICursorIterator_Members, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ICursorIterator_init, // tp_init
    PyType_GenericAlloc, // tp_alloc
    PyType_GenericNew, // tp_new
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977
    _PyObject_Del, // tp_free
    #else
    PyObject_Del, // tp_free
    #endif
    0, // tp_is_gc
    0, // tp_bases
    0, // tp_mro
    0, // tp_cache
    0, // tp_subclasses
    0, // tp_weaklist
    ICursorIterator_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ICursorIterator_Type;
}



// Implementation of the methods
int
ICursorIterator_init( PyObject* self, PyObject*  args , PyObject* /* kwds */ )
{
  coral::PyCoral::ICursorIterator* py_this = (coral::PyCoral::ICursorIterator*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );

    return -1;
  }
  py_this->parent = 0;
  if ( !PyArg_Parse( args, (char*)"O", &(py_this->parent) ) ) return -1;

  if ( py_this->parent ) Py_INCREF( py_this->parent );

  return 0;
}


void
ICursorIterator_dealloc( PyObject* self )
{
  coral::PyCoral::ICursorIterator* py_this = (coral::PyCoral::ICursorIterator*) self;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
ICursorIterator_next( PyObject* self )
{
  coral::PyCoral::ICursorIterator* py_this = (coral::PyCoral::ICursorIterator*) self;
  if( ! py_this->parent->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ICursor& theCursor = *(py_this->parent->object);

    if ( ! theCursor.next() ) return 0;

    const coral::AttributeList& theAttributeList = theCursor.currentRow();
    coral::AttributeList* element = const_cast< coral::AttributeList* >( &theAttributeList );
    coral::PyCoral::AttributeList* ob = PyObject_New( coral::PyCoral::AttributeList, coral::PyCoral::AttributeList_Type() );

    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating AttributeList object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( element, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Initialising AttributeList object." );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in iteration over Cursor" );
    return 0;
  }
}

PyObject*
ICursorIterator_iter( PyObject* self )
{
  coral::PyCoral::ICursorIterator* py_this = (coral::PyCoral::ICursorIterator*) self;
  if( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid Object" );
    return 0;
  }

  Py_INCREF(self);
  return self;
}
