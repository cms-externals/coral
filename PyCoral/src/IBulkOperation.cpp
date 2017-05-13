#include "Exception.h"
#include "IBulkOperation.h"
#include "PyCoral/cast_to_base.h"
#include "RelationalAccess/IBulkOperation.h"
#include <sstream>

// Forward declaration of the methods
static int IBulkOperation_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IBulkOperation_dealloc( PyObject* self );
//static PyObject* IBulkOperation_str( PyObject* self );

static PyObject* IBulkOperation_processNextIteration( PyObject* self );
static PyObject* IBulkOperation_flush( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::IBulkOperation_Type()
{
  static PyMethodDef IBulkOperation_Methods[] = {
    { (char*) "processNextIteration", (PyCFunction) IBulkOperation_processNextIteration, METH_NOARGS,
      (char*) "Processes the next iteration." },
    { (char*) "flush", (PyCFunction) IBulkOperation_flush, METH_NOARGS,
      (char*) "Flushes the data on the client side to the server." },
    {0, 0, 0, 0}
  };

  static char IBulkOperation_doc[] = "Interface for the execution of bulk operations.";

  static PyTypeObject IBulkOperation_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IBulkOperation", /*tp_name*/
    sizeof(coral::PyCoral::IBulkOperation), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IBulkOperation_dealloc, /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT  | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    IBulkOperation_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IBulkOperation_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IBulkOperation_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IBulkOperation_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IBulkOperation_Type;
}

int
IBulkOperation_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IBulkOperation* py_this = (coral::PyCoral::IBulkOperation*) cast_to_base( self, coral::PyCoral::IBulkOperation_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return 0;
  }
  py_this->object = 0;
  py_this->parent = 0;
  PyObject* c_object = 0;
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::IBulkOperation*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if (py_this->parent) Py_INCREF (py_this->parent);
  return 0;
}

void
IBulkOperation_dealloc( PyObject* self )
{
  coral::PyCoral::IBulkOperation* py_this = (coral::PyCoral::IBulkOperation*) cast_to_base( self, coral::PyCoral::IBulkOperation_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return ;
  }
  delete py_this->object; // Fixes memory leak bug #60017 but exposes bug #61090
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IBulkOperation_processNextIteration( PyObject* self )
{
  coral::PyCoral::IBulkOperation* py_this = (coral::PyCoral::IBulkOperation*) cast_to_base( self, coral::PyCoral::IBulkOperation_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return 0;
  }
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->processNextIteration();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when processing the next Iteration." );
    return 0;
  }
}


PyObject*
IBulkOperation_flush( PyObject* self )
{
  coral::PyCoral::IBulkOperation* py_this = (coral::PyCoral::IBulkOperation*) cast_to_base( self, coral::PyCoral::IBulkOperation_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return 0;
  }
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->flush();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when flushing the data." );
    return 0;
  }
}
