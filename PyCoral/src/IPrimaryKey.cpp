#include "Exception.h"
#include "IPrimaryKey.h"
#include "RelationalAccess/IPrimaryKey.h"
#include <sstream>

// Forward declaration of the methods
static int IPrimaryKey_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IPrimaryKey_dealloc( PyObject* self );
//static PyObject* IPrimaryKey_str( PyObject* self );

static PyObject* IPrimaryKey_columnNames( PyObject* self );
static PyObject* IPrimaryKey_tableSpaceName( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::IPrimaryKey_Type()
{
  static PyMethodDef IPrimaryKey_Methods[] = {
    { (char*) "columnNames", (PyCFunction) IPrimaryKey_columnNames, METH_NOARGS,
      (char*) "Returns the names of the columns which form the primary key." },
    { (char*) "tableSpaceName", (PyCFunction) IPrimaryKey_tableSpaceName, METH_NOARGS,
      (char*) "Returns the name of the table space where the corresponding index is created." },
    {0, 0, 0, 0}
  };

  static char IPrimaryKey_doc[] = "Interface for the description of a primary key in a table.";

  static PyTypeObject IPrimaryKey_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IPrimaryKey", /*tp_name*/
    sizeof(coral::PyCoral::IPrimaryKey), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IPrimaryKey_dealloc, /*tp_dealloc*/
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
    IPrimaryKey_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IPrimaryKey_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IPrimaryKey_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IPrimaryKey_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IPrimaryKey_Type;
}

int
IPrimaryKey_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IPrimaryKey* py_this = (coral::PyCoral::IPrimaryKey*) self;
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
  py_this->object = static_cast<coral::IPrimaryKey*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IPrimaryKey_dealloc( PyObject* self )
{
  coral::PyCoral::IPrimaryKey* py_this = (coral::PyCoral::IPrimaryKey*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IPrimaryKey_columnNames( PyObject* self )
{
  coral::PyCoral::IPrimaryKey* py_this = (coral::PyCoral::IPrimaryKey*) self;
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
                     (char*) "Error when retrieving all columnNames for the PrimaryKey." );
    return 0;
  }
}


PyObject*
IPrimaryKey_tableSpaceName( PyObject* self )
{
  coral::PyCoral::IPrimaryKey* py_this = (coral::PyCoral::IPrimaryKey*) self;
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
                     (char*) "Error when retrieving the table Space name shere corresponding index for primary is present." );
    return 0;
  }
}
