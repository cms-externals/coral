#include "Exception.h"
#include "IView.h"
#include "RelationalAccess/IView.h"
#include "IViewFactory.h"
#include "IQueryDefinition.h"
#include "RelationalAccess/IViewFactory.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
#endif
// Forward declaration of the methods
static int IViewFactory_init( PyObject* self, PyObject* /* args */, PyObject* /* kwds */);
static void IViewFactory_dealloc( PyObject* self );

static PyObject* IViewFactory_create( PyObject* self, PyObject* args );
static PyObject* IViewFactory_createOrReplace( PyObject* self, PyObject* args );

static PyObject* defineBaseClasses() {
  PyObject* baseClasses = PyTuple_New( 1 );

  PyTypeObject* IQueryDefinition_Type = coral::PyCoral::IQueryDefinition_Type();
  Py_INCREF( IQueryDefinition_Type );
  PyTuple_SET_ITEM( baseClasses, 0, (PyObject*) IQueryDefinition_Type );

  return baseClasses;
}


// Type definition
PyTypeObject*
coral::PyCoral::IViewFactory_Type()
{
  static PyMethodDef IViewFactory_Methods[] = {
    { (char*) "create", (PyCFunction)(void *) IViewFactory_create, METH_O,
      (char*) "Creates a new view with the specified name and the current query definition." },
    { (char*) "createOrReplace", (PyCFunction)(void *) IViewFactory_createOrReplace, METH_O,
      (char*) "Creates or replaces in case it exists a view with the specified name and the current query definition." },
    {0, 0, 0, 0}
  };

  static char IViewFactory_doc[] = "Interface for the definition and creation of new views in the current schema.";

  static PyObject* baseClasses = defineBaseClasses();

  static PyTypeObject IViewFactory_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IViewFactory", // tp_name
    sizeof(coral::PyCoral::IViewFactory), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IViewFactory_dealloc, // tp_dealloc
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
    IViewFactory_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IViewFactory_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IViewFactory_init, // tp_init
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
    IViewFactory_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IViewFactory_Type;
}

int
IViewFactory_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IViewFactory* py_this = (coral::PyCoral::IViewFactory*) self;
  if ( ! py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL." );
    return -1;
  }
  py_this->parent = 0;
  py_this->object = 0;
  py_this->base1 = 0;
  py_this->base1 = (PyObject*) PyObject_New( coral::PyCoral::IQueryDefinition,
                                             coral::PyCoral::IQueryDefinition_Type() );
  if ( ! py_this->base1 ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Cannot create new IQueryDefinition Object." );
    return -1;
  }
  PyObject* c_object = 0;
  if ( ! PyArg_ParseTuple( args, (char*) "OO",
                           &(py_this->parent),
                           &c_object ) ) return -1;
  py_this->object = static_cast<coral::IViewFactory*>( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  PyObject* iQueryDefinition_c_object = PyCapsule_New(static_cast<coral::IQueryDefinition*>(py_this->object), "name",0);
  Py_INCREF(Py_None);
  PyObject* temp = Py_BuildValue((char*)"OO", Py_None, iQueryDefinition_c_object );

  if ( py_this->base1->ob_type->tp_init( py_this->base1,temp,0)<0) {
    Py_DECREF(temp);
    Py_DECREF(Py_None);
    Py_DECREF(iQueryDefinition_c_object);
    py_this->object = 0;
    Py_DECREF( py_this->base1 );
    return -1;
  }
  Py_DECREF(temp);
  Py_DECREF(Py_None);
  Py_DECREF(iQueryDefinition_c_object);
  return 0;
}

void
IViewFactory_dealloc( PyObject* self )
{
  coral::PyCoral::IViewFactory* py_this = (coral::PyCoral::IViewFactory*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  if ( py_this->base1 ) {
    Py_DECREF( py_this->base1 );
  }
  self->ob_type->tp_free( self );
}


PyObject*
IViewFactory_create( PyObject* self, PyObject* args)
{
  coral::PyCoral::IViewFactory* py_this = (coral::PyCoral::IViewFactory*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* tViewName;
    if (PyString_Check(args))
      tViewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    std::string strViewName = std::string( tViewName );
    coral::IView* theCreate = const_cast<coral::IView*>( &(py_this->object->create( strViewName ) ) );
    if (! theCreate ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating underlying C++ View Object." );
      return 0;
    }
    coral::PyCoral::IView* ob = PyObject_New( coral::PyCoral::IView, coral::PyCoral::IView_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IView Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theCreate, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IView Object" );
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
                     (char*) "Error in creating a new view with the specified name and the current query definition." );
    return 0;
  }
}

PyObject*
IViewFactory_createOrReplace( PyObject* self, PyObject* args)
{
  coral::PyCoral::IViewFactory* py_this = (coral::PyCoral::IViewFactory*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* tViewName;
    if (PyString_Check(args))
      tViewName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    std::string strViewName = std::string( tViewName );
    coral::IView* theCreateOrReplace = const_cast<coral::IView*>(&( py_this->object->createOrReplace( strViewName ) ) );
    if (! theCreateOrReplace ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating underlying C++ View Object." );
      return 0;
    }
    coral::PyCoral::IView* ob = PyObject_New( coral::PyCoral::IView, coral::PyCoral::IView_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a IView Object " );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theCreateOrReplace, "name",0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IView Object" );
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
                     (char*) "Error in Creating or replacing View." );
    return 0;
  }
}
