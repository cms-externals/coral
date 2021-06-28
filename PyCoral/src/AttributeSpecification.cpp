#include "AttributeSpecification.h"
#include "Exception.h"
#include "CoralBase/AttributeSpecification.h"
#include <sstream>
#include <typeinfo>

using namespace std;

// Forward declaration of the methods
static int AttributeSpecification_init( PyObject* self , PyObject* args, PyObject* kwds);
static void AttributeSpecification_dealloc( PyObject* self );
static int AttributeSpecification_compare( PyObject* obj1, PyObject* obj2 );
#if PY_MAJOR_VERSION >= 3
static PyObject* AttributeSpecification_rich_compare( PyObject *o1, PyObject* o2, int op);
#endif
static PyObject* AttributeSpecification_str( PyObject* self );
static PyObject* AttributeSpecification_name( PyObject* self );
static PyObject* AttributeSpecification_typeName( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::AttributeSpecification_Type()
{
  static PyMethodDef AttributeSpecification_Methods[] = {
    { (char*) "name", (PyCFunction)(void *) AttributeSpecification_name, METH_NOARGS,
      (char*) "Returns the attribute name" },
    { (char*) "typeName", (PyCFunction)(void *) AttributeSpecification_typeName, METH_NOARGS,
      (char*) "Returns the type of the specification" },
    {0, 0, 0, 0}
  };

  static char AttributeSpecification_doc[] = "A class defining the specification of an Attribute.";

  static PyTypeObject AttributeSpecification_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.AttributeSpecification", // tp_name
    sizeof(coral::PyCoral::AttributeSpecification), // tp_basicsize
    0, // tp_itemsize
       //  methods
    AttributeSpecification_dealloc, // tp_dealloc
    0, // tp_print
    0, // tp_getattr
    0, // tp_setattr
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977
       AttributeSpecification_compare, // tp_compare
    #else
      NULL, // tp_compare
    #endif
    0, // tp_repr
    0, // tp_as_number
    0, // tp_as_sequence
    0, // tp_as_mapping
    0, // tp_hash
    0, // tp_call
    AttributeSpecification_str, // tp_str
    PyObject_GenericGetAttr, // tp_getattro
    PyObject_GenericSetAttr, // tp_setattro
    0, // tp_as_buffer
    Py_TPFLAGS_DEFAULT, // tp_flags
    AttributeSpecification_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977       
    0, // tp_richcompare
    #else
    (richcmpfunc)AttributeSpecification_rich_compare,
    #endif
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    AttributeSpecification_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    AttributeSpecification_init, // tp_init
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
    AttributeSpecification_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &AttributeSpecification_Type;
}

int
AttributeSpecification_init( PyObject* self, PyObject* args , PyObject* /* kwds */ )
{
  coral::PyCoral::AttributeSpecification* py_this = (coral::PyCoral::AttributeSpecification*) self;
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
  py_this->object = static_cast<coral::AttributeSpecification*>
    ( PyCapsule_GetPointer( c_object, "name" ) );
  if ( py_this->parent ) Py_INCREF (py_this->parent);

  return 0;
}


void
AttributeSpecification_dealloc( PyObject* self )
{
  coral::PyCoral::AttributeSpecification* py_this = (coral::PyCoral::AttributeSpecification*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


int
AttributeSpecification_compare( PyObject* obj1, PyObject* obj2 )
{
  coral::PyCoral::AttributeSpecification* d1 = (coral::PyCoral::AttributeSpecification*) obj1;
  coral::PyCoral::AttributeSpecification* d2 = (coral::PyCoral::AttributeSpecification*) obj2;
  if ( !d1->object || !d2->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Bad C++ Object" );
    return -1;
  }
  coral::AttributeSpecification& spec1 = *( d1->object );
  coral::AttributeSpecification& spec2 = *( d2->object );
  try {
    if (spec1 != spec2) return -1;
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not compare the AttributeSpecifications" );
    return -1;
  }
  return 0;
}

#if PY_MAJOR_VERSION >= 3
PyObject* 
AttributeSpecification_rich_compare(PyObject *o1, PyObject* o2, int op){
  if(o2==Py_None) Py_RETURN_FALSE;
  switch(op){
    case Py_EQ:{
      if(AttributeSpecification_compare(o1,o2) == 0) Py_RETURN_TRUE;
      Py_RETURN_FALSE;
    }
    default: Py_RETURN_NOTIMPLEMENTED;
  }
}
#endif


PyObject*
AttributeSpecification_str( PyObject* self )
{
  coral::PyCoral::AttributeSpecification* py_this = (coral::PyCoral::AttributeSpecification*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  std::ostringstream os;
  coral::AttributeSpecification& spec = *( py_this->object );
  os << spec.name() << "/";
  os << spec.typeName();
  std::string result = os.str();
  return Py_BuildValue( (char*) "s", result.c_str() );
}


PyObject*
AttributeSpecification_name( PyObject* self )
{
  coral::PyCoral::AttributeSpecification* py_this = (coral::PyCoral::AttributeSpecification*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::string name = py_this->object->name();
    return Py_BuildValue( (char *)"s", name.c_str() );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retrieve the name of the Attribute" );
    return 0;
  }
}



PyObject*
AttributeSpecification_typeName( PyObject* self )
{
  coral::PyCoral::AttributeSpecification* py_this = (coral::PyCoral::AttributeSpecification*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    std::string typeName = py_this->object->typeName();
    return Py_BuildValue( (char *)"s", typeName.c_str() );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retrieve the Type name of the Attribute" );
    return 0;
  }
}
