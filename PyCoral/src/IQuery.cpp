#include "Exception.h"
#include "IQuery.h"
#include "IQueryDefinition.h"
#include "ICursor.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/IQuery.h"
#include <sstream>

// Forward declaration of the methods
static int IQuery_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IQuery_dealloc( PyObject* self );
//static PyObject* IQuery_str( PyObject* self );

static PyObject* IQuery_setForUpdate( PyObject* self );
static PyObject* IQuery_setRowCacheSize( PyObject* self, PyObject* args );
static PyObject* IQuery_setMemoryCacheSize( PyObject* self, PyObject* args );
static PyObject* IQuery_defineOutputType( PyObject* self, PyObject* args );
static PyObject* IQuery_defineOutput( PyObject* self, PyObject* args );
static PyObject* IQuery_execute( PyObject* self );

static PyObject* defineBaseClasses() {
  PyObject* baseClasses = PyTuple_New( 1 );

  PyTypeObject* IQueryDefinition_Type = coral::PyCoral::IQueryDefinition_Type();
  Py_INCREF( IQueryDefinition_Type );
  PyTuple_SET_ITEM( baseClasses, 0, (PyObject*) IQueryDefinition_Type );

  return baseClasses;
}



// Type definition
PyTypeObject*
coral::PyCoral::IQuery_Type()
{
  static PyMethodDef IQuery_Methods[] = {
    { (char*) "setForUpdate", (PyCFunction) IQuery_setForUpdate, METH_NOARGS,
      (char*) "Instructs the server to lock the rows involved in the result set." },
    { (char*) "setRowCacheSize", (PyCFunction) IQuery_setRowCacheSize, METH_O,
      (char*) "Defines the client cache size in rows." },
    { (char*) "setMemoryCacheSize", (PyCFunction) IQuery_setMemoryCacheSize, METH_O,
      (char*) "Defines the client cache size in MB." },
    { (char*) "defineOutputType", (PyCFunction) IQuery_defineOutputType, METH_VARARGS,
      (char*) "Defines the output types of a given variable in the result set." },
    { (char*) "defineOutput", (PyCFunction) IQuery_defineOutput, METH_VARARGS,
      (char*) "Defines the output data buffer for the result set." },
    { (char*) "execute", (PyCFunction) IQuery_execute, METH_NOARGS,
      (char*) "Executes the query and returns a reference to the undelying ICursor object in order for the user to loop over the result set." },
    {0, 0, 0, 0}
  };

  static char IQuery_doc[] = "Interface for an executable query. Once the execute() method is called no other method can be called. Otherwise a QueryExecutedException will be thrown.";

  static PyObject* baseClasses = defineBaseClasses();

  static PyTypeObject IQuery_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IQuery", /*tp_name*/
    sizeof(coral::PyCoral::IQuery), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IQuery_dealloc, /*tp_dealloc*/
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
    IQuery_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IQuery_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IQuery_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    baseClasses, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IQuery_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IQuery_Type;
}

int
IQuery_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error self is NULL." );
    return -1;
  }
  py_this->object = 0;
  py_this->parent = 0;
  py_this->base1 = 0;
  py_this->base1 = (PyObject*) PyObject_New( coral::PyCoral::IQueryDefinition,
                                             coral::PyCoral::IQueryDefinition_Type());
  if ( ! py_this->base1 ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error IQueryDefinition object not created." );
    return -1;
  }
  PyObject* c_object =0;
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::IQuery*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if (py_this->parent) Py_INCREF(py_this->parent);
  PyObject* iqueryDefinition_c_object = PyCObject_FromVoidPtr(static_cast<coral::IQueryDefinition*>(py_this->object),0);
  Py_INCREF(Py_None);
  PyObject* temp = Py_BuildValue((char*)"OO", Py_None, iqueryDefinition_c_object );
  if (py_this->base1->ob_type->tp_init(py_this->base1,temp,0 ) < 0 ) {
    Py_DECREF(temp);
    Py_DECREF(Py_None);
    Py_DECREF(iqueryDefinition_c_object);
    py_this->object = 0;
    Py_DECREF (py_this->base1);
    return -1;
  }
  Py_DECREF(temp);
  Py_DECREF(Py_None);
  Py_DECREF(iqueryDefinition_c_object);
  return 0;
}

void
IQuery_dealloc( PyObject* self )
//destructor of the python object
{
  //get the PyCoral object from the self pointer
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  //check if we have an object
  if(py_this->object)
    //yes we have
  {
    //delete the c++ object
    delete py_this->object;
    py_this->object = 0;
  }

  if ( py_this->parent )
  {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  if ( py_this->base1 )
  {
    Py_DECREF( py_this->base1 );
  }
  self->ob_type->tp_free( self );
}


PyObject*
IQuery_setForUpdate( PyObject* self )
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->setForUpdate();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when locking the rows for updation." );
    return 0;
  }
}


PyObject*
IQuery_setRowCacheSize( PyObject* self, PyObject* args)
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setRowCacheSize( PyLong_AsLong(args) );
    }
    else if ( PyInt_Check( args ) ) {
      py_this->object->setRowCacheSize( PyInt_AS_LONG(args) );
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not a Long integer!" );
      return 0;
    }
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting row cache size at client" );
    return 0;
  }
}

PyObject*
IQuery_setMemoryCacheSize( PyObject* self, PyObject* args)
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setMemoryCacheSize( PyLong_AsLong(args) );
    }
    else if ( PyInt_Check( args ) ) {
      py_this->object->setMemoryCacheSize( PyInt_AS_LONG(args) );
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not a Long integer!" );
      return 0;
    }
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Memory cache size at client" );
    return 0;
  }
}


PyObject*
IQuery_defineOutputType( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* cppTypeName = 0;
    char* outputId = 0;
    if( ! PyArg_ParseTuple( args, (char*) "ss", &outputId, &cppTypeName ) ) return 0;
    py_this->object->defineOutputType( std::string( outputId ), std::string( cppTypeName ) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing defineOutputType operation." );
    return 0;
  }
}


PyObject*
IQuery_defineOutput( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* outputData = 0;
    if( ! PyArg_ParseTuple( args, (char*) "O!", coral::PyCoral::AttributeList_Type(), &outputData ) ) return 0;
    coral::AttributeList& theOutputData = *(outputData->object);
    py_this->object->defineOutput( theOutputData );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing defineOutput operation." );
    return 0;
  }
}


PyObject*
IQuery_execute( PyObject* self )
{
  coral::PyCoral::IQuery* py_this = (coral::PyCoral::IQuery*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ICursor* theCursor = &(py_this->object->execute()) ;
    coral::PyCoral::ICursor* ob = PyObject_New( coral::PyCoral::ICursor, coral::PyCoral::ICursor_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating cursor manager object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theCursor,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ICursor Object" );
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
                     (char*) "Error when retrieving the cursor" );
    return 0;
  }
}
