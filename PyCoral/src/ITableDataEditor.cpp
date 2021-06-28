#include "Exception.h"
#include "ITableDataEditor.h"
#include "IOperationWithQuery.h"
#include "IBulkOperation.h"
#include "IBulkOperationWithQuery.h"
#include "PyCoral/AttributeList.h"
#include "CoralBase/AttributeList.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IBulkOperationWithQuery.h"
#include <sstream>

// Forward declaration of the methods
static int ITableDataEditor_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITableDataEditor_dealloc( PyObject* self );
//static PyObject* ITableDataEditor_str( PyObject* self );

static PyObject* ITableDataEditor_rowBuffer( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_insertRow( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_bulkInsert( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_insertWithQuery( PyObject* self );
static PyObject* ITableDataEditor_bulkInsertWithQuery( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_updateRows( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_bulkUpdateRows( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_deleteRows( PyObject* self, PyObject* args );
static PyObject* ITableDataEditor_bulkDeleteRows( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::ITableDataEditor_Type()
{
  static PyMethodDef ITableDataEditor_Methods[] = {
    { (char*) "rowBuffer", (PyCFunction)(void *) ITableDataEditor_rowBuffer, METH_VARARGS,
      (char*) "Constructs a buffer corresponding to a full a table row." },
    { (char*) "insertRow", (PyCFunction)(void *) ITableDataEditor_insertRow, METH_VARARGS,
      (char*) "Inserts a new row in the table." },
    { (char*) "insertWithQuery", (PyCFunction)(void *) ITableDataEditor_insertWithQuery, METH_NOARGS,
      (char*) "Returns a new IOperationWithQuery object for performing an INSERT/SELECT operation." },
    { (char*) "bulkInsert", (PyCFunction)(void *) ITableDataEditor_bulkInsert, METH_VARARGS,
      (char*) "Returns a new IBulkOperation object for performing a bulk insert operation specifying the input data buffer and the number of rows that should be cached on the client." },
    { (char*) "bulkInsertWithQuery", (PyCFunction)(void *) ITableDataEditor_bulkInsertWithQuery, METH_O,
      (char*) "Returns a new IBulkOperationWithQuery object for performing an INSERT/SELECT operation specifying the number of iterations that should be cached on the client." },
    { (char*) "updateRows", (PyCFunction)(void *) ITableDataEditor_updateRows, METH_VARARGS,
      (char*) "Updates rows in the table. Returns the number of affected rows." },
    { (char*) "bulkUpdateRows", (PyCFunction)(void *) ITableDataEditor_bulkUpdateRows, METH_VARARGS,
      (char*) "Returns a new IBulkOperation object for performing a bulk update operation." },
    { (char*) "deleteRows", (PyCFunction)(void *) ITableDataEditor_deleteRows, METH_VARARGS,
      (char*) "Deletes the rows in the table fulfilling the specified condition. It returns the number of rows deleted." },
    { (char*) "bulkDeleteRows", (PyCFunction)(void *) ITableDataEditor_bulkDeleteRows, METH_VARARGS,
      (char*) "Returns a new IBulkOperation for peforming a bulk delete operation." },
    {0, 0, 0, 0}
  };

  static char ITableDataEditor_doc[] = "Interface for the DML operations on a table.";

  static PyTypeObject ITableDataEditor_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.ITableDataEditor", // tp_name
    sizeof(coral::PyCoral::ITableDataEditor), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ITableDataEditor_dealloc, // tp_dealloc
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
    ITableDataEditor_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    ITableDataEditor_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ITableDataEditor_init, // tp_init
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
    ITableDataEditor_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ITableDataEditor_Type;
}

int
ITableDataEditor_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
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
  py_this->object = static_cast<coral::ITableDataEditor*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if (py_this->parent) Py_INCREF (py_this->parent);
  return 0;
}


void
ITableDataEditor_dealloc( PyObject* self )
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ITableDataEditor_rowBuffer( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    if ( ! PyArg_ParseTuple(args, (char*) "O!", coral::PyCoral::AttributeList_Type(),&buffer) ) return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    py_this->object->rowBuffer(aBuffer);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving buffer corresponding to full table row" );
    return 0;
  }
}


PyObject*
ITableDataEditor_insertRow( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  coral::PyCoral::AttributeList* buffer = 0;
  try {
    if ( ! PyArg_ParseTuple(args, (char*) "O!", coral::PyCoral::AttributeList_Type(),&buffer) ) return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    py_this->object->insertRow(aBuffer);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when inserting a row" );
    return 0;
  }
}



PyObject*
ITableDataEditor_insertWithQuery( PyObject* self)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IOperationWithQuery* theInsertQuery = py_this->object->insertWithQuery();
    coral::PyCoral::IOperationWithQuery* ob = PyObject_New( coral::PyCoral::IOperationWithQuery,
                                                            coral::PyCoral::IOperationWithQuery_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IOperationWithQuery Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theInsertQuery, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IOperationWithQuery Object" );
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
                     (char*) "Error when inserting data with query" );
    return 0;
  }
}


PyObject*
ITableDataEditor_bulkInsert( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    int rowCacheSize = 0;
    if ( ! PyArg_ParseTuple(args, (char*) "O!i", coral::PyCoral::AttributeList_Type(),&buffer, &rowCacheSize) ) return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    coral::IBulkOperation* theBulkInsert = py_this->object->bulkInsert(aBuffer, rowCacheSize);
    coral::PyCoral::IBulkOperation* ob = PyObject_New( coral::PyCoral::IBulkOperation,
                                                       coral::PyCoral::IBulkOperation_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IBulkOperation Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theBulkInsert, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IBulkOperation Object" );
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
                     (char*) "Error when Bulk inserting data " );
    return 0;
  }
}



PyObject*
ITableDataEditor_bulkInsertWithQuery( PyObject* self, PyObject* args )
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int dataCacheSize = 0;
    if (PyLong_Check( args ) ) {
      dataCacheSize = PyLong_AsLong(args);
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      dataCacheSize = PyInt_AS_LONG(args);
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not a Long integer!" );
      return 0;
    }
    coral::IBulkOperationWithQuery* theBulkInsert = py_this->object->bulkInsertWithQuery(dataCacheSize);
    coral::PyCoral::IBulkOperationWithQuery* ob = PyObject_New( coral::PyCoral::IBulkOperationWithQuery,
                                                                coral::PyCoral::IBulkOperationWithQuery_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IBulkOperationWithQuery Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theBulkInsert, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IBulkOperationWithQuery Object" );
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
                     (char*) "Error when Bulk inserting data With Query" );
    return 0;
  }
}



PyObject*
ITableDataEditor_updateRows( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    char *setClause, *condition;
    if ( ! PyArg_ParseTuple(args, (char*) "ssO!", &setClause, &condition, coral::PyCoral::AttributeList_Type(),&buffer) )
      return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    long noOfRows = py_this->object->updateRows(std::string(setClause), std::string(condition), aBuffer);
    return Py_BuildValue((char*)"k", noOfRows);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when updating rows" );
    return 0;
  }
}


PyObject*
ITableDataEditor_bulkUpdateRows( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    char *setClause, *condition;
    int dataCacheSize = 0;
    if ( ! PyArg_ParseTuple(args, (char*) "ssO!i", &setClause, &condition, coral::PyCoral::AttributeList_Type(),&buffer, &dataCacheSize ) ) return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    coral::IBulkOperation* theBulkUpdate = py_this->object->bulkUpdateRows(std::string(setClause),
                                                                           std::string(condition), aBuffer, dataCacheSize);
    coral::PyCoral::IBulkOperation* ob = PyObject_New( coral::PyCoral::IBulkOperation,
                                                       coral::PyCoral::IBulkOperation_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating an IBulkOperation Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theBulkUpdate, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize Attribute Object" );
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
                     (char*) "Error when bulk updating rows" );
    return 0;
  }
}


PyObject*
ITableDataEditor_deleteRows( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    char *condition;
    if ( ! PyArg_ParseTuple(args, (char*) "sO!", &condition, coral::PyCoral::AttributeList_Type(),&buffer) ) return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    long noOfRows = py_this->object->deleteRows(std::string(condition), aBuffer);
    return Py_BuildValue((char*)"k", noOfRows);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when delete Rows" );
    return 0;
  }
}



PyObject*
ITableDataEditor_bulkDeleteRows( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITableDataEditor* py_this = (coral::PyCoral::ITableDataEditor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* buffer = 0;
    char *condition;
    int dataCacheSize = 0;
    if ( ! PyArg_ParseTuple(args, (char*) "sO!i", &condition, coral::PyCoral::AttributeList_Type(),&buffer, &dataCacheSize ) )
      return 0;
    //Py_INCREF((PyObject*)buffer);
    coral::AttributeList& aBuffer = *(buffer->object);
    coral::IBulkOperation* theBulkDelete = py_this->object->bulkDeleteRows(std::string(condition), aBuffer, dataCacheSize);
    coral::PyCoral::IBulkOperation* ob = PyObject_New( coral::PyCoral::IBulkOperation,
                                                       coral::PyCoral::IBulkOperation_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating an IBulkOperation Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theBulkDelete, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize Attribute Object" );
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
                     (char*) "Error when bulk deleting rows" );
    return 0;
  }
}
