#include "Exception.h"
#include "ITableDescription.h"
#include "IColumn.h"
#include "IPrimaryKey.h"
#include "IIndex.h"
#include "IForeignKey.h"
#include "IUniqueConstraint.h"
#include "PyCoral/cast_to_base.h"
#include "RelationalAccess/ITableDescription.h"
#include <sstream>

// Forward declaration of the methods
static int ITableDescription_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITableDescription_dealloc( PyObject* self );
//static PyObject* ITableDescription_str( PyObject* self );

static PyObject* ITableDescription_name( PyObject* self );
static PyObject* ITableDescription_type( PyObject* self );
static PyObject* ITableDescription_tableSpaceName( PyObject* self );
static PyObject* ITableDescription_numberOfColumns( PyObject* self );
static PyObject* ITableDescription_columnDescription( PyObject* self, PyObject* args );
static PyObject* ITableDescription_hasPrimaryKey( PyObject* self );
static PyObject* ITableDescription_primaryKey( PyObject* self );
static PyObject* ITableDescription_numberOfIndices( PyObject* self );
static PyObject* ITableDescription_index( PyObject* self, PyObject* args );
static PyObject* ITableDescription_numberOfForeignKeys( PyObject* self );
static PyObject* ITableDescription_foreignKey( PyObject* self, PyObject* args );
static PyObject* ITableDescription_numberOfUniqueConstraints( PyObject* self );
static PyObject* ITableDescription_uniqueConstraint( PyObject* self, PyObject* args );


// Type definition
PyTypeObject*
coral::PyCoral::ITableDescription_Type()
{
  static PyMethodDef ITableDescription_Methods[] = {
    { (char*) "name", (PyCFunction) ITableDescription_name, METH_NOARGS,
      (char*) "Returns the name of the table." },
    { (char*) "type", (PyCFunction) ITableDescription_type, METH_NOARGS,
      (char*) "Returns the table type (RDBMS SPECIFIC)." },
    { (char*) "tableSpaceName", (PyCFunction) ITableDescription_tableSpaceName, METH_NOARGS,
      (char*) "Returns the name of the table space for this table." },
    { (char*) "numberOfColumns", (PyCFunction) ITableDescription_numberOfColumns, METH_NOARGS,
      (char*) "Returns the number of columns in the table." },
    { (char*) "columnDescription", (PyCFunction) ITableDescription_columnDescription, METH_O,
      (char*) "Returns the description of the column corresponding to the specified index specified name. If the index is out of range an InvalidColumnIndexException is thrown. If the specified column name is invalid an InvalidColumnNameException is thrown" },
    { (char*) "hasPrimaryKey", (PyCFunction) ITableDescription_hasPrimaryKey, METH_NOARGS,
      (char*) "Returns the existence of a primary key in the table." },
    { (char*) "primaryKey", (PyCFunction) ITableDescription_primaryKey, METH_NOARGS,
      (char*) "Returns the primary key for the table. If there is no primary key a NoPrimaryKeyException is thrown." },
    { (char*) "numberOfIndices", (PyCFunction) ITableDescription_numberOfIndices, METH_NOARGS,
      (char*) "Returns the number of indices defined in the table." },
    { (char*) "index", (PyCFunction) ITableDescription_index, METH_O,
      (char*) "Returns the index corresponding to the specified identitier. If the identifier is out of range an InvalidIndexIdentifierException is thrown." },
    { (char*) "numberOfForeignKeys", (PyCFunction) ITableDescription_numberOfForeignKeys, METH_NOARGS,
      (char*) "Returns the number of foreign key constraints defined in the table." },
    { (char*) "foreignKey", (PyCFunction) ITableDescription_foreignKey, METH_O,
      (char*) "Returns the foreign key corresponding to the specified identifier. In case the identifier is out of range an InvalidForeignKeyIdentifierException is thrown." },
    { (char*) "numberOfUniqueConstraints", (PyCFunction) ITableDescription_numberOfUniqueConstraints, METH_NOARGS,
      (char*) "Returns the number of unique constraints defined in the table." },
    { (char*) "uniqueConstraint", (PyCFunction) ITableDescription_uniqueConstraint, METH_O,
      (char*) "Returns the unique constraint for the specified identifier. If the identifier is out of range an InvalidUniqueConstraintIdentifierException is thrown." },
    {0, 0, 0, 0}
  };

  static char ITableDescription_doc[] = "Abstract interface for the description of a relational table.";

  static PyTypeObject ITableDescription_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ITableDescription", /*tp_name*/
    sizeof(coral::PyCoral::ITableDescription), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ITableDescription_dealloc, /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    ITableDescription_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ITableDescription_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ITableDescription_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ITableDescription_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ITableDescription_Type;
}

int
ITableDescription_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base(self, coral::PyCoral::ITableDescription_Type() );
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
                          &c_object )) return -1;
  py_this->object = static_cast<coral::ITableDescription*>
    ( PyCObject_AsVoidPtr(c_object));
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}

void
ITableDescription_dealloc( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base(self, coral::PyCoral::ITableDescription_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return;
  }
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
ITableDescription_name( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type() );
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
    std::string result = py_this->object->name();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the name of the table." );
    return 0;
  }
}


PyObject*
ITableDescription_type( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    std::string result = py_this->object->type();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the type of the table." );
    return 0;
  }
}



PyObject*
ITableDescription_tableSpaceName( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    std::string result = py_this->object->tableSpaceName();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the table space name." );
    return 0;
  }
}


PyObject*
ITableDescription_numberOfColumns( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int result = py_this->object->numberOfColumns();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the numberOfColumns of table." );
    return 0;
  }
}

PyObject*
ITableDescription_columnDescription( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int columnIndex = 0;
    std::string columnName = "";
    coral::IColumn* theColumn = 0;
    if (PyLong_Check(args)) {
      columnIndex = PyLong_AsLong(args);
      theColumn = const_cast<coral::IColumn*>(&( py_this->object->columnDescription( columnIndex )));
    }
    else if (PyInt_Check(args)) {
      columnIndex = PyInt_AS_LONG(args);
      theColumn = const_cast<coral::IColumn*>(&( py_this->object->columnDescription( columnIndex )));
    }
    else if (PyString_Check(args)) {
      columnName = PyString_AS_STRING(args);
      theColumn =  const_cast<coral::IColumn*>(&( py_this->object->columnDescription( std::string(columnName) )));
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Bad Argument Type" );
      return 0;
    }
    coral::PyCoral::IColumn* ob = PyObject_New( coral::PyCoral::IColumn, coral::PyCoral::IColumn_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Column Object " );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theColumn,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IColumn Object" );
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
                     (char*) "Error when retrieving the column Description." );
    return 0;
  }
}


PyObject*
ITableDescription_hasPrimaryKey( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    if ( py_this->object->hasPrimaryKey() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving Primary Key" );
    return 0;
  }
}



PyObject*
ITableDescription_primaryKey( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    coral::IPrimaryKey* thePrimaryKey =  const_cast<coral::IPrimaryKey*>(&(py_this->object->primaryKey()));
    coral::PyCoral::IPrimaryKey* ob = PyObject_New( coral::PyCoral::IPrimaryKey, coral::PyCoral::IPrimaryKey_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Column Object " );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( thePrimaryKey,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IColumn Object" );
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
                     (char*) "Error when retrieving the Primary Key." );
    return 0;
  }
}



PyObject*
ITableDescription_numberOfIndices( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int result = py_this->object->numberOfIndices();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the numberOfIndices of table." );
    return 0;
  }
}



PyObject*
ITableDescription_index( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int indexId = 0;
    if (PyLong_Check(args)) {
      indexId = PyLong_AsLong(args);
    }
    else if (PyInt_Check(args)) {
      indexId = PyInt_AS_LONG(args);
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Bad Argument Type" );
      return 0;
    }
    coral::IIndex* theIndexId = const_cast<coral::IIndex*>(&( py_this->object->index( indexId )));
    coral::PyCoral::IIndex* ob = PyObject_New( coral::PyCoral::IIndex, coral::PyCoral::IIndex_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Index Object " );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theIndexId,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IIndex Object" );
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
                     (char*) "Error when retrieving the Index corresponding to specified Identifier." );
    return 0;
  }
}


PyObject*
ITableDescription_numberOfForeignKeys( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int result = py_this->object->numberOfForeignKeys();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the numberOfForeignKeys of table." );
    return 0;
  }
}



PyObject*
ITableDescription_numberOfUniqueConstraints( PyObject* self )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int result = py_this->object->numberOfUniqueConstraints();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the numberOfUniqueConstraints of table." );
    return 0;
  }
}



PyObject*
ITableDescription_foreignKey( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int foreignKeyId = 0;
    if (PyLong_Check(args)) {
      foreignKeyId = PyLong_AsLong(args);
    }
    else if (PyInt_Check(args)) {
      foreignKeyId = PyInt_AS_LONG(args);
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Bad Argument Type" );
      return 0;
    }
    coral::IForeignKey* theForeignKey = const_cast<coral::IForeignKey*>(&( py_this->object->foreignKey( foreignKeyId ) ));
    coral::PyCoral::IForeignKey* ob = PyObject_New( coral::PyCoral::IForeignKey, coral::PyCoral::IForeignKey_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IForeignKey Object " );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theForeignKey,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IForeignKey Object" );
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
                     (char*) "Error when retreiving foreign key" );
    return 0;
  }
}



PyObject*
ITableDescription_uniqueConstraint( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDescription* py_this = (coral::PyCoral::ITableDescription*) cast_to_base( self, coral::PyCoral::ITableDescription_Type());
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
    int uniqueConstraintId = 0;
    if (PyLong_Check(args)) {
      uniqueConstraintId = PyLong_AsLong(args);
    }
    else if (PyInt_Check(args)) {
      uniqueConstraintId = PyInt_AS_LONG(args);
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Bad Argument Type" );
      return 0;
    }
    coral::IUniqueConstraint* theUniqueConstraint = const_cast<coral::IUniqueConstraint*>
      (&( py_this->object->uniqueConstraint(uniqueConstraintId)) );
    coral::PyCoral::IUniqueConstraint* ob = PyObject_New( coral::PyCoral::IUniqueConstraint,
                                                          coral::PyCoral::IUniqueConstraint_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IUniqueConstraint Object " );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theUniqueConstraint, 0);
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IUniqueConstraint Object" );
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
                     (char*) "Error when retreiving Unique Constraint" );
    return 0;
  }
}
