#include "Exception.h"
#include "ISessionProxy.h"
#include "ISessionProperties.h"
#include "ISchema.h"
#include "ITransaction.h"
#include "ITypeConverter.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISessionProperties.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include <sstream>

// Forward declaration of the methods
static int ISessionProxy_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ISessionProxy_dealloc( PyObject* self );
//static PyObject* ISessionProxy_str( PyObject* self );

static PyObject* ISessionProxy_properties( PyObject* self );
static PyObject* ISessionProxy_nominalSchema( PyObject* self );
static PyObject* ISessionProxy_schema( PyObject* self, PyObject* args );
static PyObject* ISessionProxy_transaction( PyObject* self );
static PyObject* ISessionProxy_typeConverter( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::ISessionProxy_Type()
{
  static PyMethodDef ISessionProxy_Methods[] = {
    { (char*) "properties", (PyCFunction) ISessionProxy_properties, METH_NOARGS,
      (char*) "Returns the session info object.Since the underlying connection is probed before the object info construction, it always contains vaild data." },
    { (char*) "nominalSchema", (PyCFunction) ISessionProxy_nominalSchema, METH_NOARGS,
      (char*) "Returns the working schema of the connection." },
    { (char*) "schema", (PyCFunction) ISessionProxy_schema, METH_O,
      (char*) "Returns a reference to the ISchema object corresponding to the specified name." },
    { (char*) "transaction", (PyCFunction) ISessionProxy_transaction, METH_NOARGS,
      (char*) "Returns the transaction handle of the active session"},
    { (char*) "typeConverter", (PyCFunction) ISessionProxy_typeConverter, METH_NOARGS,
      (char*) "Returns the type converter of the active session." },
    {0, 0, 0, 0}
  };

  static char ISessionProxy_doc[] = "Interface for a proxy class to an ISession object. Whenever the user retrieves the schema, transaction or typeconverter objects, the connection is physically probed and if necessary (and being outside of an update transaction) a new connection is established. In case the connection has been broken in the middle of an update transaction, an exception is raised.";

  static PyTypeObject ISessionProxy_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ISessionProxy", /*tp_name*/
    sizeof(coral::PyCoral::ISessionProxy), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ISessionProxy_dealloc, /*tp_dealloc*/
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
    ISessionProxy_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ISessionProxy_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ISessionProxy_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ISessionProxy_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ISessionProxy_Type;
}

int
ISessionProxy_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
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
  py_this->object = static_cast<coral::ISessionProxy*>( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
ISessionProxy_dealloc( PyObject* self )
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if ( py_this->object ) delete py_this->object;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ISessionProxy_properties( PyObject* self)
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ISessionProperties* theProperties = &( py_this->object->properties() );
    coral::PyCoral::ISessionProperties* ob = PyObject_New( coral::PyCoral::ISessionProperties,
                                                           coral::PyCoral::ISessionProperties_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Session Properties object" );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theProperties,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ISessionProperties Object" );
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
                     (char*) "Error when retrieving Session properties." );
    return 0;
  }
}


PyObject*
ISessionProxy_nominalSchema( PyObject* self )
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ISchema* theNominalSchema = &( py_this->object->nominalSchema() );
    coral::PyCoral::ISchema* ob = PyObject_New( coral::PyCoral::ISchema, coral::PyCoral::ISchema_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Schema object" );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theNominalSchema,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ISchema Object" );
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
                     (char*) "Error when retrieving the Schema object" );
    return 0;
  }
}


PyObject*
ISessionProxy_schema( PyObject* self, PyObject* args )
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* schemaName;
    if (PyString_Check(args))
      schemaName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    coral::ISchema* theSchema = &( py_this->object->schema(std::string(schemaName) ) );
    coral::PyCoral::ISchema* ob = PyObject_New( coral::PyCoral::ISchema, coral::PyCoral::ISchema_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a ISchema object" );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theSchema,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ISchema Object" );
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
                     (char*) "Error when retrieving the Schema object" );
    return 0;
  }
}

PyObject*
ISessionProxy_transaction( PyObject* self)
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITransaction* theTransaction = &( py_this->object->transaction() );
    coral::PyCoral::ITransaction* ob = PyObject_New( coral::PyCoral::ITransaction, coral::PyCoral::ITransaction_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a ITransaction object" );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theTransaction,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITransaction Object" );
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
                     (char*) "Error when retrieving transaction handle of this session." );
    return 0;
  }
}


PyObject*
ISessionProxy_typeConverter( PyObject* self)
{
  coral::PyCoral::ISessionProxy* py_this = (coral::PyCoral::ISessionProxy*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITypeConverter* theTypeConverter = &( py_this->object->typeConverter() );
    coral::PyCoral::ITypeConverter* ob = PyObject_New( coral::PyCoral::ITypeConverter,
                                                       coral::PyCoral::ITypeConverter_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a TypeConverter object" );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theTypeConverter,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITypeConcerter Object" );
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
                     (char*) "Error when retrieving type converter of this session." );
    return 0;
  }
}
