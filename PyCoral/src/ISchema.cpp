#include "Exception.h"
#include "ISchema.h"
#include "IQuery.h"
#include "IView.h"
#include "ITable.h"
#include "ITableDescription.h"
#include "IViewFactory.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IViewFactory.h"
#include "cast_to_base.h"
#include "PyCoral/AttributeList.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
    #define PyString_FromString PyUnicode_FromString
#endif
// Ignore 'dereferencing type-punned pointer' warnings caused by
// Py_RETURN_TRUE/FALSE (CMS patch for sr #141482 and bug #89768)
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

// Forward declaration of the methods
static int ISchema_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ISchema_dealloc( PyObject* self );
//static PyObject* ISchema_str( PyObject* self );

static PyObject* ISchema_listTables( PyObject* self );
static PyObject* ISchema_existsTable( PyObject* self, PyObject* args );
static PyObject* ISchema_dropTable( PyObject* self, PyObject* args );
static PyObject* ISchema_truncateTable( PyObject* self, PyObject* args );
static PyObject* ISchema_dropIfExistsTable( PyObject* self, PyObject* args );
static PyObject* ISchema_createTable( PyObject* self, PyObject* args );
static PyObject* ISchema_tableHandle( PyObject* self, PyObject* args );
static PyObject* ISchema_newQuery( PyObject* self );
static PyObject* ISchema_viewFactory( PyObject* self );
static PyObject* ISchema_existsView( PyObject* self, PyObject* args );
static PyObject* ISchema_dropView( PyObject* self, PyObject* args );
static PyObject* ISchema_dropIfExistsView( PyObject* self, PyObject* args );
static PyObject* ISchema_listViews( PyObject* self );
static PyObject* ISchema_viewHandle( PyObject* self, PyObject* args );
static PyObject* ISchema_callProcedure( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::ISchema_Type()
{
  static PyMethodDef ISchema_Methods[] = {
    { (char*) "listTables", (PyCFunction)(void *) ISchema_listTables, METH_NOARGS,
      (char*) "Returns the names of all tables in the schema." },
    { (char*) "existsTable", (PyCFunction)(void *) ISchema_existsTable, METH_O,
      (char*) "Checks the existence of a table with the specified name." },
    { (char*) "dropTable", (PyCFunction)(void *) ISchema_dropTable, METH_O,
      (char*) "Drops the table with the specified name.If the table does not exist a TableNotExistingException is thrown." },
    { (char*) "truncateTable", (PyCFunction)(void *) ISchema_truncateTable, METH_O,
      (char*) "Truncates the table with the specified name.If the table does not exist a TableNotExistingException is thrown." },
    { (char*) "dropIfExistsTable", (PyCFunction)(void *) ISchema_dropIfExistsTable, METH_O,
      (char*) "Drops the table with the specified name in case it exists." },
    { (char*) "callProcedure", (PyCFunction)(void *) ISchema_callProcedure, METH_VARARGS,
      (char*) "Performs a call to a stored procedure with input parameters." },
    { (char*) "createTable", (PyCFunction)(void *) ISchema_createTable, METH_VARARGS,
      (char*) "Creates a new table with the specified description and returns the corresponding table handle. If a table with the same name already exists TableAlreadyExistingException is thrown." },
    { (char*) "tableHandle", (PyCFunction)(void *) ISchema_tableHandle, METH_O,
      (char*) "Returns a reference to an ITable object corresponding to the table with the specified name. In case no table with such a name exists, a TableNotExistingException is thrown." },
    { (char*) "newQuery", (PyCFunction)(void *) ISchema_newQuery, METH_NOARGS,
      (char*) "Returns a new query object." },
    { (char*) "viewFactory", (PyCFunction)(void *) ISchema_viewFactory, METH_NOARGS,
      (char*) "Returns a new view factory object in order to define and create a view." },
    { (char*) "existsView", (PyCFunction)(void *) ISchema_existsView, METH_O,
      (char*) "Checks the existence of a view with the specified name." },
    { (char*) "dropView", (PyCFunction)(void *) ISchema_dropView, METH_O,
      (char*) "Drops the view with the specified name. If the view does not exist a ViewNotExistingException is thrown." },
    { (char*) "dropIfExistsView", (PyCFunction)(void *) ISchema_dropIfExistsView, METH_O,
      (char*) "Drops the view with the specified name in case it exists." },
    { (char*) "listViews", (PyCFunction)(void *) ISchema_listViews, METH_NOARGS,
      (char*) "Returns the names of all views in the schema." },
    { (char*) "viewHandle", (PyCFunction)(void *) ISchema_viewHandle, METH_O,
      (char*) "Returns a reference to an IView object corresponding to the view with the specified name. In case no view with such a name exists, a ViewNotExistingException is thrown." },
    {0, 0, 0, 0}
  };

  static char ISchema_doc[] = "Abstract interface to manage a schema in a relational database. Any operation requires that a transaction has been started, otherwise a TransactionNotActiveException is thrown..";

  static PyTypeObject ISchema_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.ISchema", // tp_name
    sizeof(coral::PyCoral::ISchema), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ISchema_dealloc, // tp_dealloc
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
    ISchema_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    ISchema_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ISchema_init, // tp_init
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
    ISchema_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ISchema_Type;
}

int
ISchema_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
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
  py_this->object = static_cast<coral::ISchema*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
ISchema_dealloc( PyObject* self )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ISchema_listTables( PyObject* self )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::set<std::string> result = py_this->object->listTables();
    PyObject* py_tuple = PyTuple_New( result.size() );
    if ( !py_tuple ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not create a new PyTuple" );
      return 0;
    }
    int i=0;
    for( std::set<std::string>::const_iterator theIterator = result.begin();
         theIterator != result.end(); theIterator++ ) {
      PyObject* str = PyString_FromString( theIterator->c_str() );
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
                     (char*) "Error when retrieving all tables in the schema." );
    return 0;
  }
}


PyObject*
ISchema_existsTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tableName;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    if ( py_this->object->existsTable( std::string(tableName) ) ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when checking for a table existence" );
    return 0;
  }
}


PyObject*
ISchema_dropTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tableName;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropTable(std::string(tableName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when droping a table" );
    return 0;
  }
}


PyObject*
ISchema_truncateTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tableName;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->truncateTable(std::string(tableName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when truncating a table" );
    return 0;
  }
}


PyObject*
ISchema_callProcedure( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* procedure = 0;
    coral::PyCoral::AttributeList* inputData = 0;
    if( ! PyArg_ParseTuple( args, (char*) "sO!", &procedure, coral::PyCoral::AttributeList_Type(), &inputData ) ) return 0;
    coral::AttributeList& theInputData = *(inputData->object);
    py_this->object->callProcedure( std::string(procedure), theInputData );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when calling a procedure" );
    return 0;
  }
}



PyObject*
ISchema_dropIfExistsTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tableName;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropIfExistsTable(std::string(tableName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving exists Table query" );
    return 0;
  }
}


PyObject*
ISchema_createTable( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    PyObject* tTableDescription = 0;
    if ( ! PyArg_ParseTuple( args, (char*) "O", &tTableDescription) ) return 0;
    coral::PyCoral::ITableDescription* tableDescription = ( coral::PyCoral::ITableDescription* ) cast_to_base( tTableDescription, coral::PyCoral::ITableDescription_Type() );
    if ( ! tableDescription ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Object is not an ITableDescription object" );
      return 0;
    }
    //Py_INCREF((PyObject*)tableDescription);
    coral::ITableDescription& aTableDescription = *(tableDescription->object);
    coral::ITable* theHandle = &( py_this->object->createTable( aTableDescription ) ) ;
    if (! theHandle ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating underlying C++ ITable Object." );
      return 0;
    }
    coral::PyCoral::ITable* ob = PyObject_New( coral::PyCoral::ITable, coral::PyCoral::ITable_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating a ITable object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theHandle, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITable Object" );
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
                     (char*) "Error in Creating a table with the specified description." );
    return 0;
  }
}

PyObject*
ISchema_tableHandle( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tableName = 0;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    std::string strTableName = std::string( tableName );
    coral::ITable* theHandle = const_cast<coral::ITable*>(&( py_this->object->tableHandle( strTableName ) ) );
    if (! theHandle ) {
      PyErr_SetString( coral::PyCoral::Exception(), (char*) "Error in Creating underlying C++ Table Handle Object." );
      return 0;
    }
    coral::PyCoral::ITable* ob = PyObject_New( coral::PyCoral::ITable, coral::PyCoral::ITable_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(), (char*) "Error in Creating ITable Object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theHandle, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITable Object" );
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
                     (char*) "Error in creating a reference to a new table Handle ." );
    return 0;
  }
}


PyObject*
ISchema_newQuery( PyObject* self )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IQuery* theQuery = const_cast<coral::IQuery*>( py_this->object->newQuery() );
    if (! theQuery ) {
      PyErr_SetString( coral::PyCoral::Exception(), (char*) "Error in Creating underlying C++ IQuery Object." );
      return 0;
    }
    coral::PyCoral::IQuery* ob = PyObject_New( coral::PyCoral::IQuery, coral::PyCoral::IQuery_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating query manager object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theQuery, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IQuery Object" );
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
                     (char*) "Error when retrieving the query" );
    return 0;
  }
}

PyObject*
ISchema_viewFactory( PyObject* self )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IViewFactory* theViewFactory = const_cast<coral::IViewFactory*>( py_this->object->viewFactory() );
    if (! theViewFactory ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating underlying C++ IViewFactory Object." );
      return 0;
    }
    coral::PyCoral::IViewFactory* ob = PyObject_New( coral::PyCoral::IViewFactory, coral::PyCoral::IViewFactory_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating IViewFactory object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theViewFactory, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IViewFactory Object" );
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
                     (char*) "Error when retrieving the IViewFactory" );
    return 0;
  }
}


PyObject*
ISchema_existsView( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* viewName;
    if (PyString_Check(args))
      viewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    if ( py_this->object->existsView(std::string(viewName) ) ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving exists View query" );
    return 0;
  }
}


PyObject*
ISchema_dropView( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* viewName;
    if (PyString_Check(args))
      viewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropView(std::string(viewName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when dropping view" );
    return 0;
  }
}


PyObject*
ISchema_dropIfExistsView( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* viewName;
    if (PyString_Check(args))
      viewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->dropIfExistsView(std::string(viewName) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when executing drop dropIfExistsView query" );
    return 0;
  }
}



PyObject*
ISchema_listViews( PyObject* self )
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::set<std::string> result = py_this->object->listViews();
    PyObject* py_tuple = PyTuple_New( result.size() );
    if ( !py_tuple ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not create a new PyTuple" );
      return 0;
    }
    int i=0;
    for( std::set<std::string>::const_iterator theIterator = result.begin();
         theIterator != result.end(); theIterator++ ) {
      PyObject* str = PyString_FromString( theIterator->c_str() );
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
                     (char*) "Error when retrieving all views in the schema." );
    return 0;
  }
}



PyObject*
ISchema_viewHandle( PyObject* self, PyObject* args)
{
  coral::PyCoral::ISchema* py_this = (coral::PyCoral::ISchema*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* tViewName;
    if (PyString_Check(args))
      tViewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    std::string strViewName = std::string( tViewName );
    coral::IView* theHandle = const_cast<coral::IView*>(&( py_this->object->viewHandle( strViewName ) ) );
    if (! theHandle ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating underlying C++ View Handle Object." );
      return 0;
    }
    coral::PyCoral::IView* ob = PyObject_New( coral::PyCoral::IView, coral::PyCoral::IView_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating Iview object." );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theHandle, "name",0 );
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
                     (char*) "Error in creating a new view Handle ." );
    return 0;
  }
}
