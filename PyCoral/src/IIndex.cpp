#include "Exception.h"
#include "IIndex.h"
#include "RelationalAccess/IIndex.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyBytes_Check
    #define PyString_AS_STRING PyBytes_AS_STRING
    #define PyString_AsString PyBytes_AsString
    #define PyString_GET_SIZE PyBytes_GET_SIZE
    #define PyString_FromString PyBytes_FromString
#endif
// Ignore 'dereferencing type-punned pointer' warnings caused by
// Py_RETURN_TRUE/FALSE (CMS patch for sr #141482 and bug #89768)
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

// Forward declaration of the methods
static int IIndex_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IIndex_dealloc( PyObject* self );
//static PyObject* IIndex_str( PyObject* self );

static PyObject* IIndex_name( PyObject* self );
static PyObject* IIndex_columnNames( PyObject* self );
static PyObject* IIndex_isUnique( PyObject* self );
static PyObject* IIndex_tableSpaceName( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::IIndex_Type()
{
  static PyMethodDef IIndex_Methods[] = {
    { (char*) "name", (PyCFunction)(void *) IIndex_name, METH_NOARGS,
      (char*) "Returns the system name of the index." },
    { (char*) "columnNames", (PyCFunction)(void *) IIndex_columnNames, METH_NOARGS,
      (char*) "Returns the names of the columns which compose the index." },
    { (char*) "isUnique", (PyCFunction)(void *) IIndex_isUnique, METH_NOARGS,
      (char*) "Returns the uniqueness of the index." },
    { (char*) "tableSpaceName", (PyCFunction)(void *) IIndex_tableSpaceName, METH_NOARGS,
      (char*) "Returns the name of the table space where the index is created." },
    {0, 0, 0, 0}
  };

  static char IIndex_doc[] = "Interface describing an index on a table.";

  static PyTypeObject IIndex_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IIndex", // tp_name
    sizeof(coral::PyCoral::IIndex), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IIndex_dealloc, // tp_dealloc
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
    IIndex_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IIndex_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IIndex_init, // tp_init
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
    IIndex_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IIndex_Type;
}

int
IIndex_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
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
  py_this->object = static_cast<coral::IIndex*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IIndex_dealloc( PyObject* self )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IIndex_name( PyObject* self )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string result = py_this->object->name();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the name of the index." );
    return 0;
  }
}


PyObject*
IIndex_columnNames( PyObject* self )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::vector<std::string> result = py_this->object->columnNames();
    PyObject* py_tuple = PyTuple_New(result.size());
    if ( !py_tuple ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not create a new PyTuple" );
      return 0;
    }
    int i=0;
    for( std::vector<std::string>::const_iterator theIterator = result.begin();
         theIterator != result.end(); theIterator++ ) {
      PyObject* str = PyString_FromString(theIterator->c_str());
      PyTuple_SET_ITEM( py_tuple, i++, str );
    }
    return py_tuple;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving all columnNames for the Index." );
    return 0;
  }
}

PyObject*
IIndex_isUnique( PyObject* self )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isUnique() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is Unique for the index" );
    return 0;
  }
}


PyObject*
IIndex_tableSpaceName( PyObject* self )
{
  coral::PyCoral::IIndex* py_this = (coral::PyCoral::IIndex*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string result = py_this->object->tableSpaceName();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the table Space name where corresponding index is present." );
    return 0;
  }
}
