#include "Exception.h"
#include "IView.h"
#include "ITablePrivilegeManager.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "IColumn.h"
#include <sstream>

// Forward declaration of the methods
static int IView_init( PyObject* self, PyObject* /* args */, PyObject* /* kwds */);
static void IView_dealloc( PyObject* self );

static PyObject* IView_name( PyObject* self );
static PyObject* IView_definition( PyObject* self );
static PyObject* IView_numberOfColumns( PyObject* self );
static PyObject* IView_column( PyObject* self, PyObject* args );
static PyObject* IView_privilegeManager( PyObject* self );

//static PyObject* IView_str( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::IView_Type()
{
  static PyMethodDef IView_Methods[] = {
    { (char*) "name", (PyCFunction) IView_name, METH_NOARGS,
      (char*) "Returns the view name." },
    { (char*) "definition", (PyCFunction) IView_definition, METH_NOARGS,
      (char*) "Returns the SQL string defining the view." },
    { (char*) "numberOfColumns", (PyCFunction) IView_numberOfColumns, METH_NOARGS,
      (char*) "Returns the number of columns in the view." },
    { (char*) "column", (PyCFunction) IView_column, METH_O,
      (char*) "Returns a reference to a column description object for the specified column index." },
    { (char*) "privilegeManager", (PyCFunction) IView_privilegeManager, METH_NOARGS,
      (char*) "Returns a reference to the privilege manager of the view." },
    {0, 0, 0, 0}
  };

  static char IView_doc[] = "Interface for a View.";

  static PyTypeObject IView_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.IView", /*tp_name*/
    sizeof(coral::PyCoral::IView), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    IView_dealloc, /*tp_dealloc*/
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
    IView_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    IView_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    IView_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    IView_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &IView_Type;
}

int
IView_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
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
  py_this->object = static_cast<coral::IView*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IView_dealloc( PyObject* self )
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
IView_name( PyObject* self)
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string returnString = py_this->object->name();
    return Py_BuildValue( (char*) "s", returnString.c_str() );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in returning the view name." );
    return 0;
  }
}

PyObject*
IView_definition( PyObject* self )
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string returnString = py_this->object->definition();
    return Py_BuildValue( (char*) "s", returnString.c_str() );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in Returning the SQL string defining the view." );
    return 0;
  }
}

PyObject*
IView_numberOfColumns( PyObject* self)
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int returnValue;
    returnValue = py_this->object->numberOfColumns();
    return Py_BuildValue((char*) "i", returnValue);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error in returning the number of columns in the view." );
    return 0;
  }
}


PyObject*
IView_column( PyObject* self, PyObject* args)
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int iIndex;
    if (PyLong_Check(args)) {
      iIndex = PyLong_AsLong(args);
    }
    else if (PyInt_Check(args)) {
      iIndex = PyInt_AS_LONG(args);
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Bad Argument Type" );
      return 0;
    }
    coral::IColumn* theColumn = const_cast< coral::IColumn* >(&( py_this->object->column( iIndex ) ) );
    coral::PyCoral::IColumn* ob = PyObject_New( coral::PyCoral::IColumn, coral::PyCoral::IColumn_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in IColumn Object." );
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
                     (char*) "Error in returning a reference to a column description object for the specified column index." );
    return 0;
  }
}


PyObject*
IView_privilegeManager( PyObject* self )
{
  coral::PyCoral::IView* py_this = (coral::PyCoral::IView*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITablePrivilegeManager* thePrivilegeManager = const_cast< coral::ITablePrivilegeManager* >
      (&( py_this->object->privilegeManager( ) ) );
    coral::PyCoral::ITablePrivilegeManager* ob = PyObject_New( coral::PyCoral::ITablePrivilegeManager,
                                                               coral::PyCoral::ITablePrivilegeManager_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in creating a privilege manager object." );
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
                     (char*) "Error in returning a reference to a privilege manager." );
    return 0;
  }
}
