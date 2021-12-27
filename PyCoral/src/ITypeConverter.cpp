#include "Exception.h"
#include "ITypeConverter.h"
#include "RelationalAccess/ITypeConverter.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
    #define PyString_FromString PyUnicode_FromString
#endif
// Forward declaration of the methods
static int ITypeConverter_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITypeConverter_dealloc( PyObject* self );
//static PyObject* ITypeConverter_str( PyObject* self );

static PyObject* ITypeConverter_supportedSqlTypes( PyObject* self );
static PyObject* ITypeConverter_supportedCppTypes( PyObject* self );
static PyObject* ITypeConverter_defaultCppTypeForSqlType( PyObject* self, PyObject* args );
static PyObject* ITypeConverter_cppTypeForSqlType( PyObject* self, PyObject* args );
static PyObject* ITypeConverter_setCppTypeForSqlType( PyObject* self, PyObject* args );
static PyObject* ITypeConverter_defaultSqlTypeForCppType( PyObject* self, PyObject* args );
static PyObject* ITypeConverter_sqlTypeForCppType( PyObject* self, PyObject* args );
static PyObject* ITypeConverter_setSqlTypeForCppType( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::ITypeConverter_Type()
{
  static PyMethodDef ITypeConverter_Methods[] = {
    { (char*) "supportedSqlTypes", (PyCFunction)(void *) ITypeConverter_supportedSqlTypes, METH_NOARGS,
      (char*) "Returns the SQL types supported by the particular database server." },
    { (char*) "supportedCppTypes", (PyCFunction)(void *) ITypeConverter_supportedCppTypes, METH_NOARGS,
      (char*) "Returns the C++ types supported by the particular implementation." },
    { (char*) "defaultCppTypeForSqlType", (PyCFunction)(void *) ITypeConverter_defaultCppTypeForSqlType, METH_O,
      (char*) "Returns the default C++ type name for the given SQL type. If an invalid SQL type name is specified, an UnSupportedSqlTypeException is thrown." },
    { (char*) "cppTypeForSqlType", (PyCFunction)(void *) ITypeConverter_cppTypeForSqlType, METH_O,
      (char*) "Returns the currently registered C++ type name for the given SQL type. If an invalid SQL type name is specified, an UnSupportedSqlTypeException is thrown." },
    { (char*) "setCppTypeForSqlType", (PyCFunction)(void *) ITypeConverter_setCppTypeForSqlType, METH_VARARGS,
      (char*) "Registers a C++ type name for the given SQL type overriding the existing mapping. If any of the types specified is not supported the relevant TypeConverterException is thrown." },
    { (char*) "defaultSqlTypeForCppType", (PyCFunction)(void *) ITypeConverter_defaultSqlTypeForCppType, METH_O,
      (char*) "Returns the default SQL type name for the given C++ type. If an invalid C++ type name is specified, an UnSupportedCppTypeException is thrown." },
    { (char*) "sqlTypeForCppType", (PyCFunction)(void *) ITypeConverter_sqlTypeForCppType, METH_O,
      (char*) "Returns the currently registered SQL type name for the given C++ type. If an invalid C++ type name is specified, an UnSupportedCppTypeException is thrown." },
    { (char*) "setSqlTypeForCppType", (PyCFunction)(void *) ITypeConverter_setSqlTypeForCppType, METH_VARARGS,
      (char*) "Registers an SQL type name for the given C++ type overriding the existing mapping. If any of the types specified is not supported the relevant TypeConverterException is thrown." },
    {0, 0, 0, 0}
  };

  static char ITypeConverter_doc[] = "Abstract interface for the registry and the conversion of C++ to SQL types and vice-versa.";

  static PyTypeObject ITypeConverter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.ITypeConverter", // tp_name
    sizeof(coral::PyCoral::ITypeConverter), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ITypeConverter_dealloc, // tp_dealloc
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
    ITypeConverter_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    ITypeConverter_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ITypeConverter_init, // tp_init
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
    ITypeConverter_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ITypeConverter_Type;
}

int
ITypeConverter_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
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
  py_this->object = static_cast<coral::ITypeConverter*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
ITypeConverter_dealloc( PyObject* self )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ITypeConverter_supportedSqlTypes( PyObject* self )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::set<std::string> result = py_this->object->supportedSqlTypes();
    PyObject* py_tuple = PyTuple_New(result.size());
    if ( !py_tuple ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not create a new PyTuple" );
      return 0;
    }
    int i=0;
    for( std::set<std::string>::const_iterator theIterator = result.begin(); theIterator != result.end(); theIterator++ ) {
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
                     (char*) "Error when retrieving supported Sql Types." );
    return 0;
  }
}


PyObject*
ITypeConverter_supportedCppTypes( PyObject* self )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::set<std::string> result = py_this->object->supportedCppTypes();
    PyObject* py_tuple = PyTuple_New(result.size());
    if ( !py_tuple ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not create a new PyTuple" );
      return 0;
    }
    int i=0;
    for( std::set<std::string>::const_iterator theIterator = result.begin();
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
                     (char*) "Error when retrieving supported Cpp Types." );
    return 0;
  }
}


PyObject*
ITypeConverter_defaultCppTypeForSqlType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* defaultCpp;
    std::string theDefaultCpp;
    if (PyString_Check(args))
      defaultCpp = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    theDefaultCpp = py_this->object->defaultCppTypeForSqlType(std::string(defaultCpp) );
    return PyString_FromString(theDefaultCpp.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the default Cpp Type for Sql Type " );
    return 0;
  }
}


PyObject*
ITypeConverter_cppTypeForSqlType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* cpp;
    std::string theCpp;
    if (PyString_Check(args))
      cpp = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    theCpp = py_this->object->cppTypeForSqlType(std::string(cpp) );
    return PyString_FromString(theCpp.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the registered Cpp Type for Sql Type" );
    return 0;
  }
}


PyObject*
ITypeConverter_setCppTypeForSqlType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char *cpp, *sql;
    if ( ! PyArg_ParseTuple(args, (char*) "ss", &cpp, &sql) ) return 0;
    py_this->object->setCppTypeForSqlType(std::string(cpp), std::string(sql) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when registering Cpp Type for Sql Type" );
    return 0;
  }
}


PyObject*
ITypeConverter_defaultSqlTypeForCppType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* defaultSql;
    std::string theDefaultSql;
    if (PyString_Check(args))
      defaultSql = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    theDefaultSql = py_this->object->defaultSqlTypeForCppType(std::string(defaultSql) );
    return PyString_FromString(theDefaultSql.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the default Sql Type for Cpp Type " );
    return 0;
  }
}



PyObject*
ITypeConverter_sqlTypeForCppType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* sql;
    std::string theSql;
    if (PyString_Check(args))
      sql = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    theSql = py_this->object->sqlTypeForCppType(std::string(sql) );
    return PyString_FromString(theSql.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the registered Sql Type for Cpp Type" );
    return 0;
  }
}



PyObject*
ITypeConverter_setSqlTypeForCppType( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITypeConverter* py_this = (coral::PyCoral::ITypeConverter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char *cpp, *sql;
    if ( ! PyArg_ParseTuple(args, (char*) "ss", &sql, &cpp) ) return 0;
    py_this->object->setSqlTypeForCppType(std::string(sql), std::string(cpp));
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when registering Sql Type for Cpp Type" );
    return 0;
  }
}
