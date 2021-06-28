#include "Exception.h"
#include "IBulkOperation.h"
#include "IBulkOperationWithQuery.h"
#include "IQueryDefinition.h"
#include "RelationalAccess/IBulkOperationWithQuery.h"
#include "RelationalAccess/IQueryDefinition.h"
#include <sstream>

// Forward declaration of the methods
static int IBulkOperationWithQuery_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IBulkOperationWithQuery_dealloc( PyObject* self );
//static PyObject* IBulkOperationWithQuery_str( PyObject* self );

static PyObject* IBulkOperationWithQuery_query( PyObject* self );

static PyObject* defineBaseClasses() {
  PyObject* baseClasses = PyTuple_New( 1 );

  PyTypeObject* IBulkOperation_Type = coral::PyCoral::IBulkOperation_Type();
  Py_INCREF( IBulkOperation_Type );
  PyTuple_SET_ITEM( baseClasses, 0, (PyObject*) IBulkOperation_Type );

  return baseClasses;
}


// Type definition
PyTypeObject*
coral::PyCoral::IBulkOperationWithQuery_Type()
{
  static PyMethodDef IBulkOperationWithQuery_Methods[] = {
    { (char*) "query", (PyCFunction)(void *) IBulkOperationWithQuery_query, METH_NOARGS,
      (char*) "Returns a reference to the underlying query definition, so that it can be filled-in by the client." },
    {0, 0, 0, 0}
  };

  static char IBulkOperationWithQuery_doc[] = "Interface performing bulk DML operations where a query is involved.";

  static PyObject* baseClasses = defineBaseClasses();

  static PyTypeObject IBulkOperationWithQuery_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IBulkOperationWithQuery", // tp_name
    sizeof(coral::PyCoral::IBulkOperationWithQuery), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IBulkOperationWithQuery_dealloc, // tp_dealloc
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
    IBulkOperationWithQuery_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IBulkOperationWithQuery_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IBulkOperationWithQuery_init, // tp_init
    PyType_GenericAlloc, // tp_alloc
    PyType_GenericNew, // tp_new
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977
    _PyObject_Del, // tp_free
    #else
    PyObject_Del, // tp_free
    #endif
    0, // tp_is_gc
    baseClasses, // tp_bases
    0, // tp_mro
    0, // tp_cache
    0, // tp_subclasses
    0, // tp_weaklist
    IBulkOperationWithQuery_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IBulkOperationWithQuery_Type;
}

int
IBulkOperationWithQuery_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IBulkOperationWithQuery* py_this = (coral::PyCoral::IBulkOperationWithQuery*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }
  py_this->parent = 0;
  py_this->object = 0;
  py_this->base1 = 0;
  py_this->base1 = (PyObject*) PyObject_New( coral::PyCoral::IBulkOperation,
                                             coral::PyCoral::IBulkOperation_Type() );
  if ( ! py_this->base1 ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: IBulkOperation Object not created" );
    return -1;
  }
  PyObject* c_object = 0;
  if ( ! PyArg_ParseTuple( args, (char*) "OO",
                           &(py_this->parent),
                           &c_object) ) return -1;
  py_this->object = static_cast<coral::IBulkOperationWithQuery*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF ( py_this->parent );
  PyObject* iBulkOperation_c_object = PyCapsule_New(static_cast<coral::IBulkOperation*>(py_this->object), "name",0);
  Py_INCREF(Py_None);
  PyObject* temp = Py_BuildValue((char*)"OO", Py_None, iBulkOperation_c_object );
  if ( py_this->base1->ob_type->tp_init( py_this->base1,temp,0 ) < 0) {
    Py_DECREF(temp);
    Py_DECREF(Py_None);
    Py_DECREF(iBulkOperation_c_object);
    py_this->object = 0;
    Py_DECREF( py_this->base1 );
    return -1;
  }
  Py_DECREF(temp);
  Py_DECREF(Py_None);
  Py_DECREF(iBulkOperation_c_object);
  return 0;
}

void
IBulkOperationWithQuery_dealloc( PyObject* self )
{
  coral::PyCoral::IBulkOperationWithQuery* py_this = (coral::PyCoral::IBulkOperationWithQuery*) self;
  py_this->object = 0;
  if ( py_this->base1 ) {
    Py_DECREF( py_this->base1 );
  }

  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }

  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IBulkOperationWithQuery_query( PyObject* self )
{
  coral::PyCoral::IBulkOperationWithQuery* py_this = (coral::PyCoral::IBulkOperationWithQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IQueryDefinition* theQuery = const_cast<coral::IQueryDefinition*>(&( py_this->object->query() ));
    coral::PyCoral::IQueryDefinition* ob = PyObject_New( coral::PyCoral::IQueryDefinition,
                                                         coral::PyCoral::IQueryDefinition_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating IQueryDefinition object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theQuery, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IQueryDefinition Object" );
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
                     (char*) "Error when retrieving the Bulk Operation query" );
    return 0;
  }
}
