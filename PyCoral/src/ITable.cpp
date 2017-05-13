#include "Exception.h"
#include "ITable.h"
#include "IQuery.h"
#include "ITableDescription.h"
#include "ITableSchemaEditor.h"
#include "ITableDataEditor.h"
#include "ITablePrivilegeManager.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "PyCoral/cast_to_base.h"
#include <sstream>

// Forward declaration of the methods
static int ITable_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITable_dealloc( PyObject* self );
//static PyObject* ITable_str( PyObject* self );

static PyObject* ITable_description( PyObject* self );
static PyObject* ITable_schemaEditor( PyObject* self );
static PyObject* ITable_dataEditor( PyObject* self );
static PyObject* ITable_privilegeManager( PyObject* self );
static PyObject* ITable_newQuery( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::ITable_Type()
{
  static PyMethodDef ITable_Methods[] = {
    { (char*) "description", (PyCFunction) ITable_description, METH_NOARGS,
      (char*) "Returns the description of the table." },
    { (char*) "schemaEditor", (PyCFunction) ITable_schemaEditor, METH_NOARGS,
      (char*) "Returns a reference to the schema editor for the table." },
    { (char*) "dataEditor", (PyCFunction) ITable_dataEditor, METH_NOARGS,
      (char*) "Returns a reference to the ITableDataEditor object  for the table." },
    { (char*) "privilegeManager", (PyCFunction) ITable_privilegeManager, METH_NOARGS,
      (char*) "Returns a reference to the privilege manager of the table." },
    { (char*) "newQuery", (PyCFunction) ITable_newQuery, METH_NOARGS,
      (char*) "Returns a new query object for performing a query involving this table only." },
    {0, 0, 0, 0}
  };

  static char ITable_doc[] = "Interface for accessing and manipulating the data and the description of a relational table.";

  static PyTypeObject ITable_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ITable", /*tp_name*/
    sizeof(coral::PyCoral::ITable), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ITable_dealloc, /*tp_dealloc*/
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
    ITable_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ITable_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ITable_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ITable_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ITable_Type;
}

int
ITable_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
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
  py_this->object = static_cast<coral::ITable*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}

void
ITable_dealloc( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
ITable_description( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITableDescription* theDescription = const_cast<coral::ITableDescription*>(&( py_this->object->description() ));
    coral::PyCoral::ITableDescription* ob = PyObject_New( coral::PyCoral::ITableDescription,
                                                          coral::PyCoral::ITableDescription_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating ITableDescription object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theDescription,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITableDescription Object" );
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
                     (char*) "Error when retrieving the description of the table object" );
    return 0;
  }
}


PyObject*
ITable_schemaEditor( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITableSchemaEditor* theSchemaEditor = &( py_this->object->schemaEditor() );
    coral::PyCoral::ITableSchemaEditor* ob = PyObject_New( coral::PyCoral::ITableSchemaEditor,
                                                           coral::PyCoral::ITableSchemaEditor_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating ISchemaEditor object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theSchemaEditor,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ISchemaEditor Object" );
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
                     (char*) "Error when retrieving the schema Editor of the table object" );
    return 0;
  }
}


PyObject*
ITable_dataEditor( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITableDataEditor* theDataEditor = &( py_this->object->dataEditor() );
    coral::PyCoral::ITableDataEditor* ob = PyObject_New( coral::PyCoral::ITableDataEditor,
                                                         coral::PyCoral::ITableDataEditor_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating table data Editor object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theDataEditor,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
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
                     (char*) "Error when retrieving the data Editor of the table object" );
    return 0;
  }
}



PyObject*
ITable_privilegeManager( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITablePrivilegeManager* thePrivilegeManager = &( py_this->object->privilegeManager() );
    coral::PyCoral::ITablePrivilegeManager* ob = PyObject_New( coral::PyCoral::ITablePrivilegeManager,
                                                               coral::PyCoral::ITablePrivilegeManager_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating table privilege manager object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( thePrivilegeManager,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ITablePrivilegeManager Object" );
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
                     (char*) "Error when retrieving the privilege manager of the table object" );
    return 0;
  }
}

PyObject*
ITable_newQuery( PyObject* self )
{
  coral::PyCoral::ITable* py_this = (coral::PyCoral::ITable*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IQuery* theQuery = const_cast<coral::IQuery*>( py_this->object->newQuery() );
    coral::PyCoral::IQuery* ob = PyObject_New( coral::PyCoral::IQuery, coral::PyCoral::IQuery_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating query manager object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( (void*)theQuery,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when Initializing IQuery object" );
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
