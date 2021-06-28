#include "Exception.h"
#include "IOperationWithQuery.h"
#include "IQueryDefinition.h"
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IQueryDefinition.h"
#include <sstream>

// Forward declaration of the methods
static int IOperationWithQuery_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IOperationWithQuery_dealloc( PyObject* self );
//static PyObject* IOperationWithQuery_str( PyObject* self );

static PyObject* IOperationWithQuery_query( PyObject* self );
static PyObject* IOperationWithQuery_execute( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::IOperationWithQuery_Type()
{
  static PyMethodDef IOperationWithQuery_Methods[] = {
    { (char*) "query", (PyCFunction)(void *) IOperationWithQuery_query, METH_NOARGS,
      (char*) "Returns the description of the table." },
    { (char*) "execute", (PyCFunction)(void *) IOperationWithQuery_execute, METH_NOARGS,
      (char*) "Executes the operation and returns the number of rows affected." },
    {0, 0, 0, 0}
  };

  static char IOperationWithQuery_doc[] = "Interface for executing DML operations involving queries.";

  static PyTypeObject IOperationWithQuery_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IOperationWithQuery", // tp_name
    sizeof(coral::PyCoral::IOperationWithQuery), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IOperationWithQuery_dealloc, // tp_dealloc
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
    IOperationWithQuery_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IOperationWithQuery_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IOperationWithQuery_init, // tp_init
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
    IOperationWithQuery_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IOperationWithQuery_Type;
}

int
IOperationWithQuery_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IOperationWithQuery* py_this = (coral::PyCoral::IOperationWithQuery*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }
  py_this->object = 0;
  py_this->parent = 0;
  PyObject* c_object = 0;
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::IOperationWithQuery*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if (py_this->parent) Py_INCREF (py_this->parent);
  return 0;
}


void
IOperationWithQuery_dealloc( PyObject* self )
{
  coral::PyCoral::IOperationWithQuery* py_this = (coral::PyCoral::IOperationWithQuery*) self;
  if ( py_this->object ) delete py_this->object;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
IOperationWithQuery_query( PyObject* self )
{
  coral::PyCoral::IOperationWithQuery* py_this = (coral::PyCoral::IOperationWithQuery*) self;
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
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the query" );
    return 0;
  }
}


PyObject*
IOperationWithQuery_execute( PyObject* self )
{
  coral::PyCoral::IOperationWithQuery* py_this = (coral::PyCoral::IOperationWithQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    long result = py_this->object->execute();
    return Py_BuildValue( (char *)"k", result );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not execute the operation with query" );
    return 0;
  }
}
