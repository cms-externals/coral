#include "Exception.h"
#include "IWebCacheControl.h"
#include "RelationalAccess/IWebCacheControl.h"
#include "IWebCacheInfo.h"
#include "RelationalAccess/IWebCacheInfo.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyBytes_Check
    #define PyString_AS_STRING PyBytes_AS_STRING
    #define PyString_AsString PyBytes_AsString
    #define PyString_GET_SIZE PyBytes_GET_SIZE
    #define PyString_FromString PyBytes_FromString
#endif
// Forward declaration of the methods
static int IWebCacheControl_init( PyObject* self, PyObject* /* args */, PyObject* /* kwds */);
static void IWebCacheControl_dealloc( PyObject* self );
static PyObject* IWebCacheControl_refreshSchemaInfo( PyObject* self, PyObject* args);
static PyObject* IWebCacheControl_refreshTable( PyObject* self, PyObject* args);
static PyObject* IWebCacheControl_webCacheInfo( PyObject* self, PyObject* args);
static PyObject* IWebCacheControl_compressionLevel( PyObject* self);
static PyObject* IWebCacheControl_setCompressionLevel( PyObject* self, PyObject* args );
static PyObject* IWebCacheControl_setProxyList( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::IWebCacheControl_Type()
{
  static PyMethodDef IWebCacheControl_Methods[] = {
    { (char*) "refreshSchemaInfo", (PyCFunction)(void *) IWebCacheControl_refreshSchemaInfo, METH_VARARGS, (char*) "Instructs the RDBMS backend that all the tables within the schema \n specified by the physical or logical connection should be refreshed, in case they are accessed." },
    { (char*) "refreshTable", (PyCFunction)(void *) IWebCacheControl_refreshTable, METH_VARARGS,
      (char*) "Instructs the RDBMS backend that the specified table within the schema specified by the physical or logical connection should be refreshed in case it is accessed.  " },
    { (char*) "webCacheInfo", (PyCFunction)(void *) IWebCacheControl_webCacheInfo, METH_VARARGS,
      (char*) "Returns the web cache information for a schema given the corresponding physical or logical connection.  " },
    { (char*) "compressionLevel", (PyCFunction)(void *) IWebCacheControl_compressionLevel, METH_NOARGS,
      (char*) "Returns the previous compression level. " },
    { (char*) "setCompressionLevel", (PyCFunction)(void *) IWebCacheControl_setCompressionLevel, METH_O,
      (char*) "Sets the compression level for data transfer, 0 - off, 1 - fast, 5 - default, 9 - maximum" },
    { (char*) "setProxyList", (PyCFunction)(void *) IWebCacheControl_setProxyList, METH_VARARGS,
      (char*) "Sets the list of the web cache proxies for the fail-over mechanism" },
    {0, 0, 0, 0}
  };

  static char IWebCacheControl_doc[] = "Interface for controlling the behaviour of web caches. By default data residing on a web cache are not refreshed unless it is set otherwise through this interface..";

  static PyTypeObject IWebCacheControl_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IWebCacheControl", // tp_name
    sizeof(coral::PyCoral::IWebCacheControl), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IWebCacheControl_dealloc, // tp_dealloc
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
    IWebCacheControl_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IWebCacheControl_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IWebCacheControl_init, // tp_init
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
    IWebCacheControl_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IWebCacheControl_Type;
}

int
IWebCacheControl_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
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
  py_this->object = static_cast<coral::IWebCacheControl*>( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IWebCacheControl_dealloc( PyObject* self )
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IWebCacheControl_refreshSchemaInfo( PyObject* self, PyObject* args)
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* charConn;
    if ( ! PyArg_ParseTuple( args, (char*) "s", &charConn) ) return 0;
    std::string strConn = std::string( charConn );
    py_this->object->refreshSchemaInfo( strConn );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in refreshing schema." );
    return 0;
  }
}

PyObject*
IWebCacheControl_refreshTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* charConn;
    char* charTabName;
    if ( ! PyArg_ParseTuple( args, (char*) "ss", &charConn, &charTabName) ) return 0;
    std::string strConn = std::string( charConn );
    std::string strTabName = std::string( charTabName );
    py_this->object->refreshTable( strConn, strTabName );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in refreshing table." );
    return 0;
  }
}


PyObject*
IWebCacheControl_webCacheInfo( PyObject* self, PyObject* args)
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IWebCacheInfo *webCacheInfo = 0;
    char* charConn;
    if ( ! PyArg_ParseTuple( args, (char*) "s", &charConn) ) return 0;
    std::string strConn = std::string( charConn );
    webCacheInfo = const_cast<coral::IWebCacheInfo*>(&( py_this->object->webCacheInfo( strConn ) ) );
    coral::PyCoral::IWebCacheInfo* ob = PyObject_New( coral::PyCoral::IWebCacheInfo, coral::PyCoral::IWebCacheInfo_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error while creating WebCacheInfo object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( webCacheInfo, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IWebCacheInfo Object" );
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
                     (char*) "Error in returning the web cache information." );
    return 0;
  }
}

PyObject*
IWebCacheControl_compressionLevel( PyObject* self)
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int returnValue;
    returnValue = py_this->object->compressionLevel( );
    return Py_BuildValue( (char*) "i", &returnValue );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in returning the previous compression level." );
    return 0;
  }
}

PyObject*
IWebCacheControl_setCompressionLevel( PyObject* self, PyObject* args )
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setCompressionLevel( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setCompressionLevel( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Bad Argument Type !" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in Seting the compression level." );
    return 0;
  }
  Py_RETURN_NONE;
}

PyObject*
IWebCacheControl_setProxyList( PyObject* self, PyObject* args )
{
  coral::PyCoral::IWebCacheControl* py_this = (coral::PyCoral::IWebCacheControl*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    PyObject* proxyListTuple = 0;
    PyObject* aProxyList = 0;
    if ( ! PyArg_ParseTuple(args, (char*) "O", &proxyListTuple) ) return 0;
    //Py_INCREF( proxyListTuple );
    int noOfColumns = PyTuple_GET_SIZE( proxyListTuple );
    std::vector <std::string> str_Vector;
    std::string strData = "";
    if ( !PyTuple_CheckExact(proxyListTuple) )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: The proxyList is not in tuple format" );
      return 0;
    }
    for ( int i=0; i< noOfColumns; i++) {
      aProxyList = PyTuple_GET_ITEM( proxyListTuple, i);
      if (PyString_Check(aProxyList))
      {
        strData = std::string(PyString_AS_STRING(aProxyList));
      } else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: Tuple contents are not string" );
        return 0;
      }
      str_Vector.push_back( strData );
    }
    //Py_DECREF( proxyListTuple );
    py_this->object->setProxyList( str_Vector );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in Seting the list of the web cache proxies." );
    return 0;
  }
  Py_RETURN_NONE;
}
