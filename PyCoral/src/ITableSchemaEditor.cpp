#include "Python.h" // Include Python.h before system headers (fix bug #73166)
#include <iostream>
#include <sstream>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "PyCoral/cast_to_base.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "Exception.h"
#include "ITableSchemaEditor.h"

// Forward declaration of the methods
static int ITableSchemaEditor_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITableSchemaEditor_dealloc( PyObject* self );
//static PyObject* ITableSchemaEditor_str( PyObject* self );

static PyObject* ITableSchemaEditor_insertColumn( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_dropColumn( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_renameColumn( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_changeColumnType( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_setNotNullConstraint( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_setUniqueConstraint( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_setPrimaryKey( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_dropPrimaryKey( PyObject* self );
static PyObject* ITableSchemaEditor_createIndex( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_dropIndex( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_createForeignKey( PyObject* self, PyObject* args );
static PyObject* ITableSchemaEditor_dropForeignKey( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::ITableSchemaEditor_Type()
{
  static PyMethodDef ITableSchemaEditor_Methods[] = {
    { (char*) "insertColumn", (PyCFunction) ITableSchemaEditor_insertColumn, METH_VARARGS,
      (char*) "Inserts a new column in the table. If the column name already exists or is invalid, an InvalidColumnNameException is thrown." },
    { (char*) "dropColumn", (PyCFunction) ITableSchemaEditor_dropColumn, METH_O,
      (char*) "Drops a column from the table. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown." },
    { (char*) "renameColumn", (PyCFunction) ITableSchemaEditor_renameColumn, METH_VARARGS,
      (char*) "Renames a column in the table. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown." },
    { (char*) "changeColumnType", (PyCFunction) ITableSchemaEditor_changeColumnType, METH_VARARGS,
      (char*) "Changes the C++ type of a column in the table. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown." },
    { (char*) "setNotNullConstraint", (PyCFunction) ITableSchemaEditor_setNotNullConstraint, METH_VARARGS,
      (char*) "Sets or removes a NOT NULL constraint on a column. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown." },
    { (char*) "setUniqueConstraint", (PyCFunction) ITableSchemaEditor_setUniqueConstraint, METH_VARARGS,
      (char*) "Adds or removes a unique constraint on a column. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown. If a unique constrain already exists for the specified column an UniqueConstraintAlreadyExistingException is thrown. If not unique constraint exists for the specified column in the case it is asked to be dropped, an InvalidUniqueConstraintIdentifierException is thrown ." },
    { (char*) "setPrimaryKey", (PyCFunction) ITableSchemaEditor_setPrimaryKey, METH_VARARGS,
      (char*) "Defines a primary key from a single column. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown. If a primary key has already been defined, an ExistingPrimaryKeyException is thrown." },
    { (char*) "dropPrimaryKey", (PyCFunction) ITableSchemaEditor_dropPrimaryKey, METH_NOARGS,
      (char*) "Drops the existing primary key. If there is no primary key defined a NoPrimaryKeyException is thrown." },
    { (char*) "createIndex", (PyCFunction) ITableSchemaEditor_createIndex, METH_VARARGS,
      (char*) "Creates an index on a column. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown. If an index has already been defined with that name an InvalidIndexIdentifierException is thrown." },
    { (char*) "dropIndex", (PyCFunction) ITableSchemaEditor_dropIndex, METH_O,
      (char*) "Drops an existing index. If the specified index name is not valid an InvalidIndexIdentifierException is thrown." },
    { (char*) "createForeignKey", (PyCFunction) ITableSchemaEditor_createForeignKey, METH_VARARGS,
      (char*) "Creates a foreign key constraint. If the column name does not exist or is invalid, an InvalidColumnNameException is thrown. If a foreign key has already been defined with that name an InvalidForeignKeyIdentifierException is thrown." },
    { (char*) "dropForeignKey", (PyCFunction) ITableSchemaEditor_dropForeignKey, METH_O,
      (char*) "Drops a foreign key. If the specified name is not valid an InvalidForeignKeyIdentifierException is thrown." },
    {0, 0, 0, 0}
  };

  static char ITableSchemaEditor_doc[] = "Interface for altering the schema of an existing table.";

  static PyTypeObject ITableSchemaEditor_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ITableSchemaEditor", /*tp_name*/
    sizeof(coral::PyCoral::ITableSchemaEditor), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ITableSchemaEditor_dealloc, /*tp_dealloc*/
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
    ITableSchemaEditor_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ITableSchemaEditor_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ITableSchemaEditor_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ITableSchemaEditor_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ITableSchemaEditor_Type;
}

int
ITableSchemaEditor_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::ITableSchemaEditor*>( PyCObject_AsVoidPtr( c_object ) );
  if (py_this->parent) Py_INCREF(py_this->parent);
  return 0;
}

void
ITableSchemaEditor_dealloc( PyObject* self )
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
ITableSchemaEditor_insertColumn( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments < 2 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Bad Arguments" );
      return 0;
    }
    char *columnName = 0;
    char *typeName = 0;
    char *tableSpaceName = (char*)"";
    int size = 0;
    PyObject* fixedSize = Py_True;
    Py_INCREF(Py_True);
    if ( ! PyArg_ParseTuple(args, (char*) "ss|iOs", &columnName, &typeName, &size, &fixedSize, &tableSpaceName) ) {
      Py_DECREF(Py_True);
      return 0;
    }
    py_this->object->insertColumn(std::string(columnName), std::string(typeName), size, ( PyObject_IsTrue( fixedSize ) ? true : false ), std::string(tableSpaceName) );
    Py_DECREF(Py_True);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when inserting column" );
    return 0;
  }
}


PyObject*
ITableSchemaEditor_dropColumn( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    char *tableSchemaName;
    if (PyString_Check(args))
      tableSchemaName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropColumn(std::string(tableSchemaName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping column from the Table schema" );
    return 0;
  }
}



PyObject*
ITableSchemaEditor_renameColumn( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    char *originalName, *newName;
    if ( ! PyArg_ParseTuple(args, (char*) "ss", &originalName, &newName) ) return 0;
    py_this->object->renameColumn(std::string(originalName), std::string(newName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping column from the Table schema" );
    return 0;
  }
}


PyObject*
ITableSchemaEditor_changeColumnType( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when changing the column type" );
      return 0;
    }
    char *columnName = 0;
    char *typeName = 0;
    int size = 0;
    PyObject* fixedSize = Py_True;
    Py_INCREF(Py_True);
    if ( ! PyArg_ParseTuple(args, (char*) "ss|iO", &columnName, &typeName, &size, &fixedSize) ) {
      Py_DECREF(Py_True);
      return 0;
    }
    py_this->object->changeColumnType(std::string(columnName), std::string(typeName), size, ( PyObject_IsTrue( fixedSize ) ? true : false ) );
    Py_DECREF(Py_True);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when changing the column type" );
    return 0;
  }
}


PyObject*
ITableSchemaEditor_setNotNullConstraint( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when setNotNullConstraint, Lack of Argument" );
      return 0;
    }
    char *columnName = 0;
    PyObject* isNotNull = Py_True;
    Py_INCREF(Py_True);
    if ( ! PyArg_ParseTuple(args, (char*) "s|O", &columnName, &isNotNull) ) {
      Py_DECREF(Py_True);
      return 0;
    }
    py_this->object->setNotNullConstraint(std::string(columnName), ( PyObject_IsTrue( isNotNull ) ? true : false ) );
    Py_DECREF(Py_True);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Not Null Constraint" );
    return 0;
  }
}

PyObject*
ITableSchemaEditor_setUniqueConstraint( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when setUniqueConstraint, Lack of Argument" );
      return 0;
    }
    char *columnName = 0;
    PyObject* columnTuple = 0;
    PyObject* aColumn = 0;
    char *name = (char*)"";
    PyObject* isUnique = Py_True;
    Py_INCREF(Py_True);
    char *tableSpaceName = (char*)"";
    if ( PyTuple_Check(PyTuple_GET_ITEM(args,0)) ) {
      if ( ! PyArg_ParseTuple(args, (char*) "O|sOs", &columnTuple, &name, &isUnique, &tableSpaceName) ) {
        Py_DECREF(Py_True);
        return 0;
      }
      //Py_INCREF( columnTuple );
      int noOfColumns = PyTuple_GET_SIZE( columnTuple );
      std::vector <std::string> str_Vector;
      std::string strData = "";
      if ( !PyTuple_Check(columnTuple) )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: The columns are not in tuple format" );
        Py_DECREF(Py_True);
        return 0;
      }
      for ( int i=0; i< noOfColumns; i++) {
        aColumn = PyTuple_GET_ITEM( columnTuple, i);
        if (PyString_Check(aColumn))
        {
          strData = std::string(PyString_AS_STRING(aColumn));
        } else {
          PyErr_SetString( coral::PyCoral::Exception(),
                           (char*) "Error: Tuple contents are not string" );
          Py_DECREF(Py_True);
          return 0;
        }
        str_Vector.push_back( strData );
      }
      //Py_DECREF( columnTuple );
      py_this->object->setUniqueConstraint( str_Vector, std::string(name), ( PyObject_IsTrue( isUnique ) ? true : false ), std::string(tableSpaceName ));
      Py_DECREF(Py_True);
    } else if ( PyString_Check( PyTuple_GET_ITEM(args,0)) ) {
      if ( ! PyArg_ParseTuple(args, (char*) "s|sOs", &columnName, &name, &isUnique, &tableSpaceName) ) {
        Py_DECREF(Py_True);
        return 0;
      }
      py_this->object->setUniqueConstraint( std::string(columnName), std::string(name), ( PyObject_IsTrue( isUnique ) ? true : false ), std::string(tableSpaceName ));
      Py_DECREF(Py_True);
    } else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: The columns are not in appropriate format" );
      Py_DECREF(Py_True);
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Unique Constraint" );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
ITableSchemaEditor_setPrimaryKey( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when setPrimary, Lack of Argument" );
      return 0;
    }
    char *columnName = 0;
    PyObject* columnTuple = 0;
    PyObject* aColumn = 0;
    char *tableSpaceName = (char*)"";
    if ( PyTuple_Check(PyTuple_GET_ITEM(args,0)) ) {
      if ( ! PyArg_ParseTuple(args, (char*) "O|s", &columnTuple, &tableSpaceName) ) return 0;
      //Py_INCREF( columnTuple );
      int noOfColumns = PyTuple_GET_SIZE( columnTuple );
      std::vector <std::string> str_Vector;
      std::string strData = "";
      if ( !PyTuple_Check(columnTuple) )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: The columns are not in tuple format" );
        return 0;
      }
      for ( int i=0; i< noOfColumns; i++) {
        aColumn = PyTuple_GET_ITEM( columnTuple, i);
        if (PyString_Check(aColumn))
        {
          strData = std::string(PyString_AS_STRING(aColumn));
        } else {
          PyErr_SetString( coral::PyCoral::Exception(),
                           (char*) "Error: Tuple contents are not string" );
          return 0;
        }
        str_Vector.push_back( strData );
      }
      //Py_DECREF( columnTuple );
      py_this->object->setPrimaryKey( str_Vector, std::string(tableSpaceName) );
    } else if ( PyString_Check( PyTuple_GET_ITEM(args,0)) ) {
      if ( ! PyArg_ParseTuple(args, (char*) "s|s", &columnName, &tableSpaceName) ) return 0;
      py_this->object->setPrimaryKey(std::string(columnName), std::string(tableSpaceName) );
    } else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: The columns are not in appripriate format" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Primary Key" );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
ITableSchemaEditor_dropPrimaryKey( PyObject* self)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    py_this->object->dropPrimaryKey();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping primary key" );
    return 0;
  }
}

PyObject*
ITableSchemaEditor_createIndex( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments < 2 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating Index, Lack of Argument" );
      return 0;
    }
    char *indexName = 0;
    char *columnName = 0;
    PyObject* columnTuple = 0;
    PyObject* aColumn = 0;
    PyObject* isUnique = Py_False;
    Py_INCREF(Py_False);
    char *tableSpaceName = (char*)"";
    if ( PyTuple_Check(PyTuple_GET_ITEM(args,1)) ) {
      if ( ! PyArg_ParseTuple(args, (char*) "sO|Os", &indexName, &columnTuple, &isUnique, &tableSpaceName) ) {
        Py_DECREF(Py_False);
        return 0;
      }
      //Py_INCREF( columnTuple );
      int noOfColumns = PyTuple_GET_SIZE( columnTuple );
      std::vector <std::string> str_Vector;
      std::string strData = "";
      if ( !PyTuple_Check(columnTuple) )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: The columns are not in tuple format" );
        Py_DECREF(Py_False);
        return 0;
      }
      for ( int i=0; i< noOfColumns; i++) {
        aColumn = PyTuple_GET_ITEM( columnTuple, i);
        if (PyString_Check(aColumn))
        {
          strData = std::string(PyString_AS_STRING(aColumn));
        } else {
          PyErr_SetString( coral::PyCoral::Exception(),
                           (char*) "Error: Tuple contents are not string" );
          Py_DECREF(Py_False);
          return 0;
        }
        str_Vector.push_back( strData );
      }
      //Py_DECREF( columnTuple );
      py_this->object->createIndex( std::string(indexName),
                                    str_Vector, ( PyObject_IsTrue( isUnique ) ? true : false ), std::string(tableSpaceName ));
      Py_DECREF(Py_False);
    } else if ( PyString_Check(PyTuple_GET_ITEM(args,1))) {
      if ( ! PyArg_ParseTuple(args, (char*) "ss|Os", &indexName, &columnName, &isUnique, &tableSpaceName) ) {
        Py_DECREF(Py_False);
        return 0;
      } else {
        py_this->object->createIndex(std::string(indexName),
                                     std::string(columnName), ( PyObject_IsTrue( isUnique ) ? true : false ), std::string(tableSpaceName) );
        Py_DECREF(Py_False);
      }
    }else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: The columns are not in tuple format OR in string format" );
      Py_DECREF(Py_False);
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when creating Index" );
    return 0;
  }
  //Py_DECREF(Py_False);
  Py_RETURN_NONE;
}


PyObject*
ITableSchemaEditor_dropIndex( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    char *indexName;
    if (PyString_Check(args))
      indexName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropIndex( std::string( indexName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping an existing index." );
    return 0;
  }
}


PyObject*
ITableSchemaEditor_createForeignKey( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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

  try
  {
    int numberOfArguments = PyTuple_GET_SIZE( args );
#ifdef CORAL240DC
    if ( numberOfArguments < 5 )
#else
      if ( numberOfArguments < 4 )
#endif
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error when creating ForeignKey, Lack of Argument" );
        return 0;
      }

    char *name = 0;
    char *columnName = 0;
    PyObject* columnTuple = 0;
    PyObject* aColumn = 0;
    char *refTableName = 0;
    char *refColumnName = 0;
    PyObject* refColumnTuple = 0;
#ifdef CORAL240DC
    bool refOnDeleteCascade = 0;
#endif
    PyObject* aRefColumn = 0;
    if (( PyTuple_Check(PyTuple_GET_ITEM(args,1)) ) && ( PyTuple_Check(PyTuple_GET_ITEM(args,3)) ) )
    {
#ifdef CORAL240DC
      if ( ! PyArg_ParseTuple(args, (char*) "sOsOb", &name, &columnTuple, &refTableName, &refColumnTuple, &refOnDeleteCascade) ) return 0;
#else
      if ( ! PyArg_ParseTuple(args, (char*) "sOsO", &name, &columnTuple, &refTableName, &refColumnTuple) ) return 0;
#endif

      std::cout << "schring" << std::endl;

      //Py_INCREF( columnTuple );
      int noOfColumns = PyTuple_GET_SIZE( columnTuple );
      std::vector <std::string> str_Vector;
      std::string strData = "";
      if ( !PyTuple_Check(columnTuple) )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: The columns are not in tuple format" );
        return 0;
      }
      for ( int i=0; i< noOfColumns; i++) {
        aColumn = PyTuple_GET_ITEM( columnTuple, i);
        if (PyString_Check(aColumn))
        {
          strData = std::string(PyString_AS_STRING(aColumn));
        } else {
          PyErr_SetString( coral::PyCoral::Exception(),
                           (char*) "Error: Tuple contents are not string" );
          return 0;
        }
        str_Vector.push_back( strData );
      }
      //Py_DECREF( columnTuple );
      //Py_INCREF( refColumnTuple );
      int noOfRefColumns = PyTuple_GET_SIZE( refColumnTuple );
      std::vector <std::string> ref_str_Vector;
      std::string ref_strData = "";
      if ( !PyTuple_Check(refColumnTuple) )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: The columns are not in tuple format" );
        return 0;
      }
      for ( int i=0; i< noOfRefColumns; i++) {
        aRefColumn = PyTuple_GET_ITEM( refColumnTuple, i);
        if (PyString_Check(aRefColumn))
        {
          ref_strData = std::string(PyString_AS_STRING(aRefColumn));
        } else {
          PyErr_SetString( coral::PyCoral::Exception(),
                           (char*) "Error: Tuple contents are not string" );
          return 0;
        }
        ref_str_Vector.push_back( ref_strData );
      }
      //Py_DECREF( refColumnTuple );
#ifdef CORAL240DC
      py_this->object->createForeignKey( std::string(name),
                                         str_Vector, std::string(refTableName ), ref_str_Vector, refOnDeleteCascade );
#else
      py_this->object->createForeignKey( std::string(name),
                                         str_Vector, std::string(refTableName ), ref_str_Vector );
#endif

    }
    else if (( PyString_Check(PyTuple_GET_ITEM(args,1))) && ( PyString_Check(PyTuple_GET_ITEM(args,3))))
    {
#ifdef CORAL240DC
      if ( ! PyArg_ParseTuple(args, (char*) "ssssb", &name, &columnName, &refTableName, &refColumnName, &refOnDeleteCascade) ) return 0;

      py_this->object->createForeignKey(std::string(name),
                                        std::string(columnName), std::string(refTableName), std::string(refColumnName), refOnDeleteCascade );
#else
      if ( ! PyArg_ParseTuple(args, (char*) "ssss", &name, &columnName, &refTableName, &refColumnName) ) return 0;

      py_this->object->createForeignKey(std::string(name),
                                        std::string(columnName), std::string(refTableName), std::string(refColumnName) );
#endif


    }
    else
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: The columns are not in appripriate format" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when creating Foreign Key" );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
ITableSchemaEditor_dropForeignKey( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableSchemaEditor* py_this = (coral::PyCoral::ITableSchemaEditor*) cast_to_base( self, coral::PyCoral::ITableSchemaEditor_Type());
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
    char *name;
    if (PyString_Check(args))
      name = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropForeignKey( std::string( name ) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping a foreign key." );
    return 0;
  }
}
