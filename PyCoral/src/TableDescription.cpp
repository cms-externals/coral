#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#include "Python.h" // Include Python.h before system headers (fix bug #73166 aka SPI-209)
#pragma clang diagnostic pop

#include <memory>
#include <sstream>
#include "Exception.h"
#include "TableDescription.h"
#include "IColumn.h"
#include "IForeignKey.h"
#include "IIndex.h"
#include "IPrimaryKey.h"
#include "ITableDescription.h"
#include "ITableSchemaEditor.h"
#include "IUniqueConstraint.h"
#include "RelationalAccess/ITableDescription.h"
#include "RelationalAccess/ITableSchemaEditor.h"
#include "RelationalAccess/TableDescription.h"
#include "cast_to_base.h"

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
#endif
// Forward declaration of the methods
static int TableDescription_init( PyObject* self, PyObject* args, PyObject* kwds );
static void TableDescription_dealloc( PyObject* self );
//static PyObject* TableDescription_str( PyObject* self );

static PyObject* TableDescription_setName( PyObject* self, PyObject* args );
static PyObject* TableDescription_setType( PyObject* self, PyObject* args );
static PyObject* TableDescription_setTableSpaceName( PyObject* self, PyObject* args );

static PyObject* defineBaseClasses() {
  PyObject* baseClasses = PyTuple_New( 2 );

  PyTypeObject* ITableSchemaEditor_Type = coral::PyCoral::ITableSchemaEditor_Type();
  Py_INCREF( ITableSchemaEditor_Type );
  PyTuple_SET_ITEM( baseClasses, 0, (PyObject*) ITableSchemaEditor_Type );

  PyTypeObject* ITableDescription_Type = coral::PyCoral::ITableDescription_Type();
  Py_INCREF( ITableDescription_Type );
  PyTuple_SET_ITEM( baseClasses, 1, (PyObject*) ITableDescription_Type );

  return baseClasses;
}


// Type definition
PyTypeObject*
coral::PyCoral::TableDescription_Type()
{
  static PyMethodDef TableDescription_Methods[] = {
    { (char*) "setName", (PyCFunction)(void *) TableDescription_setName, METH_O,
      (char*) "Sets the name of the table." },
    { (char*) "setType", (PyCFunction)(void *) TableDescription_setType, METH_O,
      (char*) "Sets the type of the table." },
    { (char*) "setTableSpaceName", (PyCFunction)(void *) TableDescription_setTableSpaceName, METH_O,
      (char*) "Sets the name of the table space." },
    {0, 0, 0, 0}
  };

  static char TableDescription_doc[] = "Abstract interface for the description of a relational table.";

  static PyObject* baseClasses = defineBaseClasses();

  static PyTypeObject TableDescription_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.TableDescription", // tp_name
    sizeof(coral::PyCoral::TableDescription), // tp_basicsize
    0, // tp_itemsize
       //  methods
    TableDescription_dealloc, // tp_dealloc
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
    TableDescription_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    TableDescription_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    TableDescription_init, // tp_init
    PyType_GenericAlloc, // tp_alloc
    PyType_GenericNew, // tp_new
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977
    _PyObject_Del, // tp_free
    #else
    PyObject_Del, // tp_free
    #endif
    0, // tp_is_gc
    baseClasses, // tp_bases
    0, // tp_mro
    0, // tp_cache
    0, // tp_subclasses
    0, // tp_weaklist
    TableDescription_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &TableDescription_Type;
}

int
TableDescription_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::TableDescription* py_this = (coral::PyCoral::TableDescription*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }
  py_this->base1 = 0;
  py_this->base2 = 0;
  py_this->object = 0;
  py_this->base1 = (PyObject*) PyObject_New( coral::PyCoral::ITableSchemaEditor,
                                             coral::PyCoral::ITableSchemaEditor_Type() );
  if ( !py_this->base1 ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Could not create ITableSchemaEditor Object" );
    return -1;
  }
  int numberOfArguments = PyTuple_GET_SIZE(args);
  std::unique_ptr<coral::TableDescription> tableDescription; // Fix Coverity RESOURCE_LEAK (bug #95668)
  coral::PyCoral::ITableDescription* iTableDesc = 0;
  switch (numberOfArguments) {
  default:
    tableDescription.reset( new coral::TableDescription );
    break;
  case 1:
    if ( PyString_Check(PyTuple_GET_ITEM(args,0)) ) {
      tableDescription.reset( new coral::TableDescription(std::string(PyString_AsString(PyTuple_GET_ITEM(args,0)))) );
    } else {
      if ( ! PyArg_ParseTuple(args,(char*)"O!",coral::PyCoral::ITableDescription_Type(),&iTableDesc)) return 0;
      coral::ITableDescription& iTableDescription = *(static_cast<coral::ITableDescription*>(iTableDesc->object));
      tableDescription.reset( new coral::TableDescription(iTableDescription) );
    }
    break;
  case 2:
    char* context = 0;
    if ( ! PyArg_ParseTuple(args,(char*)"O!s",coral::PyCoral::ITableDescription_Type(),&iTableDesc,&context)) return 0;
    coral::ITableDescription& iTableDescription = *(static_cast<coral::ITableDescription*>(iTableDesc->object));
    tableDescription.reset( new coral::TableDescription(iTableDescription, std::string(context)) );
    break;
  }
  py_this->base2 = (PyObject*) PyObject_New( coral::PyCoral::ITableDescription,
                                             coral::PyCoral::ITableDescription_Type() );
  if ( !py_this->base2 ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Could not create ITableDescription Object" );
    Py_DECREF( py_this->base1 );
    return -1;
  }
  coral::TableDescription* pTableDescription = tableDescription.release(); // Fix Coverity RESOURCE_LEAK (bug #95668) and WRAPPER_ESCAPE
  py_this->object = pTableDescription;
  coral::ITableSchemaEditor* editor = static_cast< coral::ITableSchemaEditor* >( pTableDescription );
  coral::ITableDescription* description = static_cast< coral::ITableDescription* >( pTableDescription );
  PyObject* editor_c_object = PyCapsule_New( editor, "name", 0);
  PyObject* description_c_object = PyCapsule_New( description, "name", 0);
  Py_INCREF(Py_None);
  PyObject* temp1 = Py_BuildValue((char*)"OO", Py_None, editor_c_object );
  Py_INCREF(Py_None);
  PyObject* temp2 = Py_BuildValue((char*)"OO", Py_None, description_c_object );
  if ( py_this->base1->ob_type->tp_init( py_this->base1,temp1,0)<0 ||
       py_this->base2->ob_type->tp_init( py_this->base2,temp2,0)<0) {
    Py_DECREF(temp1);
    Py_DECREF(temp2);
    Py_DECREF(Py_None);
    Py_DECREF(Py_None);
    Py_DECREF(editor_c_object);
    Py_DECREF(description_c_object);
    delete py_this->object;
    py_this->object = 0;
    return -1;
  }
  Py_DECREF(temp1);
  Py_DECREF(temp2);
  Py_DECREF(Py_None);
  Py_DECREF(Py_None);
  Py_DECREF(editor_c_object);
  Py_DECREF(description_c_object);
  return 0;
}


void
TableDescription_dealloc( PyObject* self )
{
  coral::PyCoral::TableDescription* py_this = (coral::PyCoral::TableDescription*) self;
  if( py_this->object ) delete py_this->object;
  py_this->object = 0;
  if ( py_this->base1 ) {
    Py_DECREF( py_this->base1 );
  }
  if ( py_this->base2 ) {
    Py_DECREF( py_this->base2 );
  }
  self->ob_type->tp_free( self );
}


PyObject*
TableDescription_setName( PyObject* self, PyObject* args)
{
  coral::PyCoral::TableDescription* py_this = (coral::PyCoral::TableDescription*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* tableName;
    if (PyString_Check(args))
      tableName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setName(std::string(tableName));
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when trying to set table name" );
    return 0;
  }
}


PyObject*
TableDescription_setType( PyObject* self, PyObject* args)
{
  coral::PyCoral::TableDescription* py_this = (coral::PyCoral::TableDescription*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* tableType;
    if (PyString_Check(args))
      tableType = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setType(std::string(tableType));
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when trying to set table type" );
    return 0;
  }
}

PyObject*
TableDescription_setTableSpaceName( PyObject* self, PyObject* args)
{
  coral::PyCoral::TableDescription* py_this = (coral::PyCoral::TableDescription*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* tableSpaceName;
    if (PyString_Check(args))
      tableSpaceName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setTableSpaceName(std::string(tableSpaceName));
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when trying to set table space name" );
    return 0;
  }
}
