#include "Exception.h"
#include "IColumn.h"
#include "RelationalAccess/IColumn.h"
#include <sstream>

// Forward declaration of the methods
static int IColumn_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IColumn_dealloc( PyObject* self );
//static PyObject* IColumn_str( PyObject* self );

static PyObject* IColumn_name( PyObject* self );
static PyObject* IColumn_type( PyObject* self );
static PyObject* IColumn_indexInTable( PyObject* self );
static PyObject* IColumn_isNotNull( PyObject* self );
static PyObject* IColumn_isUnique( PyObject* self );
static PyObject* IColumn_size( PyObject* self );
static PyObject* IColumn_isSizeFixed( PyObject* self );
static PyObject* IColumn_tableSpaceName( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::IColumn_Type()
{
  static PyMethodDef IColumn_Methods[] = {
    { (char*) "name", (PyCFunction) IColumn_name, METH_NOARGS,
      (char*) "Returns the name of the column." },
    { (char*) "type", (PyCFunction) IColumn_type, METH_NOARGS,
      (char*) "Returns the C++ type of the column." },
    { (char*) "indexInTable", (PyCFunction) IColumn_indexInTable, METH_NOARGS,
      (char*) "Returns the column id in the table." },
    { (char*) "isNotNull", (PyCFunction) IColumn_isNotNull, METH_NOARGS,
      (char*) "Returns the NOT-NULL-ness of the column." },
    { (char*) "isUnique", (PyCFunction) IColumn_isUnique, METH_NOARGS,
      (char*) "Returns the uniqueness of the column." },
    { (char*) "size", (PyCFunction) IColumn_size, METH_NOARGS,
      (char*) "Returns the maximum size in bytes of the data object which can be held in this column." },
    { (char*) "isSizeFixed", (PyCFunction) IColumn_isSizeFixed, METH_NOARGS,
      (char*) "Informs whether the size of the object is fixed or it can be variable. This makes sense mostly for string types." },
    { (char*) "tableSpaceName", (PyCFunction) IColumn_tableSpaceName, METH_NOARGS,
      (char*) "Returns the name of table space for the data. This makes sence mainly for LOBs." },
    {0, 0, 0, 0}
  };

  static char IColumn_doc[] = "Interface for the description of a column in a table.";

  static PyTypeObject IColumn_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IColumn", /*tp_name*/
    sizeof(coral::PyCoral::IColumn), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IColumn_dealloc, /*tp_dealloc*/
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
    IColumn_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IColumn_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IColumn_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IColumn_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IColumn_Type;
}

int
IColumn_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error self is NULL" );
    return -1;
  }
  py_this->object = 0;
  py_this->parent = 0;
  PyObject* c_object = 0;
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::IColumn*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );

  return 0;
}


void
IColumn_dealloc( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }

  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IColumn_name( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
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
                     (char*) "Error when retrieving the name of the column." );
    return 0;
  }
}


PyObject*
IColumn_type( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string result = py_this->object->type();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the type of the column." );
    return 0;
  }
}


PyObject*
IColumn_indexInTable( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = py_this->object->indexInTable();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the index of the column." );
    return 0;
  }
}


PyObject*
IColumn_isNotNull( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isNotNull() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is Not Null for the column" );
    return 0;
  }
}


PyObject*
IColumn_isUnique( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
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
                     (char*) "Error when retrieving is Unique for the column" );
    return 0;
  }
}


PyObject*
IColumn_size( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    long result = py_this->object->size();
    return Py_BuildValue((char*)"k",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the size of the column." );
    return 0;
  }
}


PyObject*
IColumn_isSizeFixed( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isSizeFixed() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is Size Fixed for the column" );
    return 0;
  }
}


PyObject*
IColumn_tableSpaceName( PyObject* self )
{
  coral::PyCoral::IColumn* py_this = (coral::PyCoral::IColumn*) self;
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
                     (char*) "Error when retrieving the table Space name of the column." );
    return 0;
  }
}
