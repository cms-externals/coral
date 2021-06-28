#include "Exception.h"
#include "ISessionProperties.h"
#include "RelationalAccess/ISessionProperties.h"
#include <sstream>

// Forward declaration of the methods
static int ISessionProperties_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ISessionProperties_dealloc( PyObject* self );
//static PyObject* ISessionProperties_str( PyObject* self );

static PyObject* ISessionProperties_flavorName( PyObject* self );
static PyObject* ISessionProperties_serverVersion( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::ISessionProperties_Type()
{
  static PyMethodDef ISessionProperties_Methods[] = {
    { (char*) "flavorName", (PyCFunction)(void *) ISessionProperties_flavorName, METH_NOARGS,
      (char*) "Returns the name of the RDBMS flavour." },
    { (char*) "serverVersion", (PyCFunction)(void *) ISessionProperties_serverVersion, METH_NOARGS,
      (char*) "Returns the version of the database server." },
    {0, 0, 0, 0}
  };

  static char ISessionProperties_doc[] = "Interface providing session info to the related ISessionProxy.";

  static PyTypeObject ISessionProperties_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.ISessionProperties", // tp_name
    sizeof(coral::PyCoral::ISessionProperties), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ISessionProperties_dealloc, // tp_dealloc
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
    ISessionProperties_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    ISessionProperties_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ISessionProperties_init, // tp_init
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
    ISessionProperties_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ISessionProperties_Type;
}

int
ISessionProperties_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ISessionProperties* py_this = (coral::PyCoral::ISessionProperties*) self;
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
  py_this->object = static_cast<coral::ISessionProperties*>
    ( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
ISessionProperties_dealloc( PyObject* self )
{
  coral::PyCoral::ISessionProperties* py_this = (coral::PyCoral::ISessionProperties*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ISessionProperties_flavorName( PyObject* self )
{
  coral::PyCoral::ISessionProperties* py_this = (coral::PyCoral::ISessionProperties*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string result = py_this->object->flavorName();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the name of RDBMS flavor." );
    return 0;
  }
}


PyObject*
ISessionProperties_serverVersion( PyObject* self )
{
  coral::PyCoral::ISessionProperties* py_this = (coral::PyCoral::ISessionProperties*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string result = py_this->object->serverVersion();
    return Py_BuildValue((char*)"s",result.c_str());
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the version of the database server." );
    return 0;
  }
}
