#include "Exception.h"
#include "IQueryDefinition.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "CoralBase/AttributeList.h"
#include "cast_to_base.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
#endif
// Forward declaration of the methods
static int IQueryDefinition_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IQueryDefinition_dealloc( PyObject* self );
//static PyObject* IQueryDefinition_str( PyObject* self );

static PyObject* IQueryDefinition_setDistinct( PyObject* self );
static PyObject* IQueryDefinition_addToOutputList( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_addToTableList( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_defineSubQuery( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_setCondition( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_addToOrderList( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_groupBy( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_limitReturnedRows( PyObject* self, PyObject* args );
static PyObject* IQueryDefinition_applySetOperation( PyObject* self, PyObject* args );


// Type definition
PyTypeObject*
coral::PyCoral::IQueryDefinition_Type()
{
  static PyMethodDef IQueryDefinition_Methods[] = {
    { (char*) "setDistinct", (PyCFunction)(void *) IQueryDefinition_setDistinct, METH_NOARGS,
      (char*) "Requires a distinct selection." },
    { (char*) "addToOutputList", (PyCFunction)(void *) IQueryDefinition_addToOutputList, METH_VARARGS,
      (char*) "Appends an expression to the output list." },
    { (char*) "addToTableList", (PyCFunction)(void *) IQueryDefinition_addToTableList, METH_VARARGS,
      (char*) "Appends a table name in the table selection list (the FROM part of the query)." },
    { (char*) "defineSubQuery", (PyCFunction)(void *) IQueryDefinition_defineSubQuery, METH_O,
      (char*) "Defines a subquery. The specified name should be used in a subsequent call to addToTableList." },
    { (char*) "setCondition", (PyCFunction)(void *) IQueryDefinition_setCondition, METH_VARARGS,
      (char*) "Defines the condition to the query (the WHERE clause)." },
    { (char*) "groupBy", (PyCFunction)(void *) IQueryDefinition_groupBy, METH_O,
      (char*) "Appends a GROUP BY clause in the query ." },
    { (char*) "addToOrderList", (PyCFunction)(void *) IQueryDefinition_addToOrderList, METH_O,
      (char*) "Appends an expression to the ordering list (the ORDER clause) ." },
    { (char*) "limitReturnedRows", (PyCFunction)(void *) IQueryDefinition_limitReturnedRows, METH_VARARGS,
      (char*) "Instructs the server to send only up to maxRows rows at the result of the query starting from the offset row." },
    { (char*) "applySetOperation", (PyCFunction)(void *) IQueryDefinition_applySetOperation, METH_VARARGS,
      (char*) "Applies a set operation. Returns the rhs query definition so that it can be filled ." },
    {0, 0, 0, 0}
  };

  static char IQueryDefinition_doc[] = "Interface for the definition of a query.";

  static PyTypeObject IQueryDefinition_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IQueryDefinition", // tp_name
    sizeof(coral::PyCoral::IQueryDefinition), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IQueryDefinition_dealloc, // tp_dealloc
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
    Py_TPFLAGS_DEFAULT  | Py_TPFLAGS_BASETYPE, // tp_flags
    IQueryDefinition_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IQueryDefinition_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IQueryDefinition_init, // tp_init
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
    IQueryDefinition_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IQueryDefinition_Type;
}



int
IQueryDefinition_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return -1;
  }
  py_this->object = 0;
  py_this->parent = 0;
  PyObject* c_object = 0;
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::IQueryDefinition*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if (py_this->parent) Py_INCREF (py_this->parent);
  return 0;
}

void
IQueryDefinition_dealloc( PyObject* self )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error casting to base." );
    return ;
  }
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IQueryDefinition_setDistinct( PyObject* self )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    py_this->object->setDistinct();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing set distinct operation." );
    return 0;
  }
}



PyObject*
IQueryDefinition_addToOutputList( PyObject* self, PyObject* args)
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    char* expression = 0;
    char* alias = (char*)"";
    if( ! PyArg_ParseTuple( args, (char*) "s|s", &expression, &alias ) ) return 0;
    py_this->object->addToOutputList(std::string(expression), std::string(alias) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing addToOutputList operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_addToTableList( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    char* tableName = 0;
    char* alias = (char*)"";
    if( ! PyArg_ParseTuple( args, (char*) "s|s", &tableName, &alias ) ) return 0;
    py_this->object->addToTableList(std::string(tableName), std::string(alias));
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing addToTableList operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_defineSubQuery( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    const char* alias = 0;
    if (PyString_Check(args))
      alias = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type16" );
      return 0;
    }
    coral::IQueryDefinition* theQueryDefinition=0;
    theQueryDefinition = &(py_this->object->defineSubQuery( std::string( alias ) ));
    coral::PyCoral::IQueryDefinition* ob = PyObject_New( coral::PyCoral::IQueryDefinition,
                                                         coral::PyCoral::IQueryDefinition_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IQueryDefinition object" );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theQueryDefinition, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IQueryDefinition Object" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing defineSubQuery operation." );
    return 0;
  }
}



PyObject*
IQueryDefinition_setCondition( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    char* condition = 0;
    coral::PyCoral::AttributeList* inputData = 0;
    if( ! PyArg_ParseTuple( args, (char*) "sO!", &condition, coral::PyCoral::AttributeList_Type(), &inputData ) ) return 0;
    //Py_INCREF(inputData);
    coral::AttributeList& theInputData = *(inputData->object);
    py_this->object->setCondition( std::string(condition), theInputData );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing setCondition operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_groupBy( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    const char* expression = 0;
    if (PyString_Check(args))
      expression = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type17" );
      return 0;
    }
    py_this->object->groupBy( std::string(expression) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing groupBy operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_addToOrderList( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    const char* expression = 0;
    if (PyString_Check(args))
      expression = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type18" );
      return 0;
    }
    py_this->object->addToOrderList( std::string(expression) );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing addToOrderList operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_limitReturnedRows( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    int maxRows = 0;
    int offset = 0;
    if( ! PyArg_ParseTuple( args, (char*) "|ii",&maxRows, &offset ) ) return 0;
    py_this->object->limitReturnedRows( maxRows, offset );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing limitReturnedRows operation." );
    return 0;
  }
}


PyObject*
IQueryDefinition_applySetOperation( PyObject* self, PyObject* args )
{
  coral::PyCoral::IQueryDefinition* py_this = (coral::PyCoral::IQueryDefinition*) cast_to_base( self, coral::PyCoral::IQueryDefinition_Type());
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
    coral::IQueryDefinition::SetOperation operationType = coral::IQueryDefinition::Union;
    if ( ! PyArg_ParseTuple(args, (char*)"i", &operationType)) return 0;
    coral::IQueryDefinition* theSetOperation=0;
    theSetOperation = &(py_this->object->applySetOperation( operationType ));
    coral::PyCoral::IQueryDefinition* ob = PyObject_New( coral::PyCoral::IQueryDefinition,
                                                         coral::PyCoral::IQueryDefinition_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IQueryDefinition object" );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theSetOperation, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IqueryDefinition Object" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when performing applySet operation." );
    return 0;
  }
}
