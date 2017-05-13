#include "Exception.h"
#include "ITransaction.h"
#include "RelationalAccess/ITransaction.h"
#include <sstream>

// Forward declaration of the methods
static int ITransaction_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITransaction_dealloc( PyObject* self );
//static PyObject* ITransaction_str( PyObject* self );

static PyObject* ITransaction_start( PyObject* self, PyObject* args );
static PyObject* ITransaction_commit( PyObject* self );
static PyObject* ITransaction_rollback( PyObject* self );
static PyObject* ITransaction_isActive( PyObject* self );
static PyObject* ITransaction_isReadOnly( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::ITransaction_Type()
{
  static PyMethodDef ITransaction_Methods[] = {
    { (char*) "start", (PyCFunction) ITransaction_start, METH_VARARGS,
      (char*) "Starts a new transaction.In case of failure a TransactionNotStartedException is thrown." },
    { (char*) "commit", (PyCFunction) ITransaction_commit, METH_NOARGS,
      (char*) "Commits the transaction. In case of failure a TransactionNotCommittedException is thrown." },
    { (char*) "rollback", (PyCFunction) ITransaction_rollback, METH_NOARGS,
      (char*) "Aborts and rolls back a transaction." },
    { (char*) "isActive", (PyCFunction) ITransaction_isActive, METH_NOARGS,
      (char*) "Returns the status of the transaction (if it is active or not)." },
    { (char*) "isReadOnly", (PyCFunction) ITransaction_isReadOnly, METH_NOARGS,
      (char*) "Returns the mode of the transaction (if it is read-only or not)." },
    {0, 0, 0, 0}
  };

  static char ITransaction_doc[] = "Interface for the transaction control in an active session.";

  static PyTypeObject ITransaction_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ITransaction", /*tp_name*/
    sizeof(coral::PyCoral::ITransaction), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ITransaction_dealloc, /*tp_dealloc*/
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
    ITransaction_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ITransaction_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ITransaction_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ITransaction_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ITransaction_Type;
}

int
ITransaction_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
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
  py_this->object = static_cast<coral::ITransaction*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
ITransaction_dealloc( PyObject* self )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ITransaction_start( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if (numberOfArguments == 0) {
      py_this->object->start();
    } else {
      PyObject *boolObject = 0;
      if( ! PyArg_ParseTuple(args, (char*) "O", &boolObject ) ) return 0;
      py_this->object->start(( PyObject_IsTrue( boolObject ) ? true : false ));
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when starting the transaction." );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
ITransaction_commit( PyObject* self )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->commit();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when committing the transaction." );
    return 0;
  }
}


PyObject*
ITransaction_rollback( PyObject* self )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->rollback();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when rolling back the transaction." );
    return 0;
  }
}


PyObject*
ITransaction_isActive( PyObject* self )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isActive() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is Active for the transaction" );
    return 0;
  }
}


PyObject*
ITransaction_isReadOnly( PyObject* self )
{
  coral::PyCoral::ITransaction* py_this = (coral::PyCoral::ITransaction*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isReadOnly() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is ReadOnly for the transaction" );
    return 0;
  }
}
