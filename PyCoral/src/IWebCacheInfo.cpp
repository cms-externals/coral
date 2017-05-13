#include "Exception.h"
#include "IWebCacheInfo.h"
#include "RelationalAccess/IWebCacheInfo.h"
#include <sstream>

// Get rid of 'dereferencing type-punned pointer will break strict-aliasing rules'
// warnings caused by Py_RETURN_TRUE/FALSE.
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

// Forward declaration of the methods
static int IWebCacheInfo_init( PyObject* self, PyObject* /* args */, PyObject* /* kwds */);
static void IWebCacheInfo_dealloc( PyObject* self );
static PyObject* IWebCacheInfo_isSchemaInfoCached( PyObject* self );
static PyObject* IWebCacheInfo_isTableCached( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::IWebCacheInfo_Type()
{
  static PyMethodDef IWebCacheInfo_Methods[] = {
    { (char*) "isSchemaInfoCached", (PyCFunction) IWebCacheInfo_isSchemaInfoCached, METH_NOARGS,
      (char*) "Checks if the schema info (data dictionary) is cached, i.e.it  does not need to be refreshed" },
    { (char*) "isTableCached", (PyCFunction) IWebCacheInfo_isTableCached, METH_O,
      (char*) "Checks if a table in the schema is cached, i.e.it  does not need to be refreshed" },
    {0, 0, 0, 0}
  };

  static char IWebCacheInfo_doc[] = "Interface for accessing the web cache policy for a given schema.";

  static PyTypeObject IWebCacheInfo_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IWebCacheInfo", /*tp_name*/
    sizeof(coral::PyCoral::IWebCacheInfo), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IWebCacheInfo_dealloc, /*tp_dealloc*/
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
    IWebCacheInfo_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IWebCacheInfo_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IWebCacheInfo_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IWebCacheInfo_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IWebCacheInfo_Type;
}

int
IWebCacheInfo_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IWebCacheInfo* py_this = (coral::PyCoral::IWebCacheInfo*) self;
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
  py_this->object = static_cast<coral::IWebCacheInfo*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IWebCacheInfo_dealloc( PyObject* self )
{
  coral::PyCoral::IWebCacheInfo* py_this = (coral::PyCoral::IWebCacheInfo*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IWebCacheInfo_isSchemaInfoCached( PyObject* self )
{
  coral::PyCoral::IWebCacheInfo* py_this = (coral::PyCoral::IWebCacheInfo*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if( py_this->object->isSchemaInfoCached() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in checking schema info." );
    return 0;
  }
}

PyObject*
IWebCacheInfo_isTableCached( PyObject* self, PyObject* args)
{
  coral::PyCoral::IWebCacheInfo* py_this = (coral::PyCoral::IWebCacheInfo*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* charTableName;
    if (PyString_Check(args))
      charTableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    std::string strTabName = std::string( charTableName );
    if( py_this->object->isTableCached( strTabName ) ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in checking table." );
    return 0;
  }
}
