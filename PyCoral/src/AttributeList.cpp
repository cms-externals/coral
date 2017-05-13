#include "Exception.h"
#include "PyCoral/AttributeList.h"
#include "AttributeListIterator.h"
#include "Attribute.h"
#include "AttributeSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeSpecification.h"
#include <sstream>
#include <typeinfo>


// Forward declaration of the methods
static int AttributeList_init( PyObject* self, PyObject* args, PyObject* kwds );
static void AttributeList_dealloc( PyObject* self );
static int AttributeList_compare( PyObject* obj1, PyObject* obj2 );
static PyObject* AttributeList_str( PyObject* self );
static PyObject* AttributeList_iter( PyObject* self );
static PyObject* AttributeList_attributeByIndex( PyObject* self, int index );
static PyObject* AttributeList_attributeByName( PyObject* self, PyObject* args );
static int AttributeList_length( PyObject* self );



static PyObject* AttributeList_extend( PyObject* self, PyObject* args );
static PyObject* AttributeList_size( PyObject* self );
static PyObject* AttributeList_copyData( PyObject* self, PyObject* args );
static PyObject* AttributeList_fastCopyData( PyObject* self, PyObject* args );
static PyObject* AttributeList_merge( PyObject* self, PyObject* args );
static int AttributeList_length( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::AttributeList_Type()
{
  static PyMethodDef AttributeList_Methods[] = {
    { (char*) "extend", (PyCFunction) AttributeList_extend, METH_VARARGS,
      (char*) "Extends the attribute list by one attribute, given the specification" },
    { (char*) "copyData", (PyCFunction) AttributeList_copyData, METH_VARARGS,
      (char*) "Copies the data of an  attribute list to another, performs typechecking, rhs can have longer specification, the first attributes are used" },
    { (char*) "fastCopyData", (PyCFunction) AttributeList_fastCopyData, METH_VARARGS,
      (char*) "Copies the data of an  attribute list to another, does not perform typechecking, rhs can have longer specification, the first attributes are used" },
    { (char*) "merge", (PyCFunction) AttributeList_merge, METH_VARARGS,
      (char*) "Merges into and shares the contents of another attribute list" },
    { (char*) "size", (PyCFunction) AttributeList_size, METH_NOARGS,
      (char*) "Returns the size of attribute list" },
    {0, 0, 0, 0}
  };

  static PyMappingMethods AttributeList_MappingMethods = {
#if PY_VERSION_HEX >= 0x02050000
    (lenfunc) AttributeList_length, /* length */
#else
    AttributeList_length, /* length */
#endif
    AttributeList_attributeByName, /* get attribute */
    0
  };


  static char AttributeList_doc[] = "A simple container of attributes.";

  static PyTypeObject AttributeList_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.AttributeList", /*tp_name*/
    sizeof(coral::PyCoral::AttributeList), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    AttributeList_dealloc, /*tp_dealloc*/
    0, /*tp_print*/
    0, /*tp_getattr*/
    0, /*tp_setattr*/
    AttributeList_compare, /*tp_compare*/
    0, /*tp_repr*/
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    &AttributeList_MappingMethods, /*tp_as_mapping*/
    0, /*tp_hash*/
    0, /*tp_call*/
    AttributeList_str, /*tp_str*/
    PyObject_GenericGetAttr, /*tp_getattro*/
    PyObject_GenericSetAttr, /*tp_setattro*/
    0, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    AttributeList_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    AttributeList_iter, /*tp_iter*/
    0, /*tp_iternext*/
    AttributeList_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    AttributeList_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    AttributeList_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &AttributeList_Type;
}

int
AttributeList_init( PyObject* self, PyObject*  args , PyObject* /* kwds */ )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;

  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }

  py_this->object = 0;
  int numberOfArguments = PyTuple_GET_SIZE( args );
  try {
    if (numberOfArguments == 0) {
      py_this->object = new coral::AttributeList();
      py_this->parent = 0;
    }
    else {
      PyObject* c_object = 0;
      if ( !PyArg_ParseTuple( args, (char*)"OO",
                              &(py_this->parent),
                              &c_object ) ) return -1;
      py_this->object = static_cast<coral::AttributeList*>
        ( PyCObject_AsVoidPtr( c_object ) );
      if ( py_this->parent ) Py_INCREF( py_this->parent );
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     "Could not create a new AttributeList" );
    return -1;
  }
  return 0;
}


void
AttributeList_dealloc( PyObject* self )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if ( py_this->object && ! py_this->parent ) {
    delete py_this->object;
    py_this->object = 0;
  }
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
    py_this->parent = 0;
  }
  self->ob_type->tp_free( self );
}


int
AttributeList_compare( PyObject* obj1, PyObject* obj2 )
{
  coral::PyCoral::AttributeList* d1 = (coral::PyCoral::AttributeList*) obj1;
  coral::PyCoral::AttributeList* d2 = (coral::PyCoral::AttributeList*) obj2;
  if ( !d1->object || !d2->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Bad C++ Object" );
    return -1;
  }
  coral::AttributeList& list1 = *( d1->object );
  coral::AttributeList& list2 = *( d2->object );
  try {
    if (list1 != list2) return -1;
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not compare the AttributeList" );
    return -1;
  }
  return 0;
}


PyObject*
AttributeList_str( PyObject* self )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  std::ostringstream os;
  coral::AttributeList& attrlist = *( py_this->object );
  attrlist.toOutputStream( os );
  std::string result = os.str();
  return Py_BuildValue( (char*) "s", result.c_str() );
}


PyObject*
AttributeList_extend( PyObject* self, PyObject* args )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try{
    char* name = 0;
    char* typeName = 0;
    if( ! PyArg_ParseTuple( args, (char *)"ss", &name , &typeName ) ) return 0;
    std::string tName= std::string(typeName);
    const std::type_info* tNameInfo = coral::AttributeSpecification::typeIdForName(tName);
    if ( tNameInfo ) {
      py_this->object->extend( std::string(name), *tNameInfo );
      Py_RETURN_NONE;
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Invalid Type Name supplied, not extending list" );
      return 0;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not extend the AttributeList" );
    return 0;
  }
}


PyObject*
AttributeList_size( PyObject* self )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    long result = py_this->object->size();
    return Py_BuildValue( (char *)"l", result );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Could not retrieve the size of AttributeList" );
    return 0;
  }
}

PyObject*
AttributeList_copyData( PyObject* self, PyObject* args )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::PyCoral::AttributeList* attrList = 0;
    if( ! PyArg_ParseTuple( args, (char *)"O!", coral::PyCoral::AttributeList_Type() , &attrList ) ) return 0;
    //Py_INCREF( (PyObject*) attrList );
    if ( attrList->object ) {
      if ( py_this->object->size() <= attrList->object->size())
        py_this->object->copyData( *(attrList->object) );
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Error: Cannot CopyData, Target AttributeList Size is greater than the source" );
      }
      Py_RETURN_NONE;
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not copy the data of an empty AttributeList" );
      return 0;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not copy the data of AttributeList" );
    return 0;
  }
}


PyObject*
AttributeList_fastCopyData( PyObject* self, PyObject* args )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  coral::PyCoral::AttributeList *attrList = 0;
  try {
    if( ! PyArg_ParseTuple( args, (char *)"O!", coral::PyCoral::AttributeList_Type() , &attrList ) ) return 0;
    //Py_INCREF( (PyObject*) attrList );
    if ( attrList->object ) {
      if ( py_this->object->size() <= attrList->object->size())
        py_this->object->fastCopyData( *(attrList->object) );
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Error: Cannot FastCopy, Target AttributeList Size is greater than the source" );
      }
      Py_RETURN_NONE;
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not fast copy the data of an empty AttributeList" );
      return 0;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Could not fast copy the data of AttributeList" );
    return 0;
  }
}


PyObject*
AttributeList_merge( PyObject* self, PyObject* args )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try{
    coral::PyCoral::AttributeList* attrList = 0;
    if( ! PyArg_ParseTuple( args, (char *)"O!", coral::PyCoral::AttributeList_Type() , &attrList ) ) return 0;
    //Py_INCREF( (PyObject*) attrList );
    if ( attrList->object ) {
      py_this->object->merge( *(attrList->object) );
      Py_RETURN_NONE;
    }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not merge the data of an empty AttributeList" );
      return 0;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not merge the data of AttributeList" );
    return 0;
  }
}


PyObject*
AttributeList_iter( PyObject* self )
{
  try{
    coral::PyCoral::AttributeListIterator* ob = PyObject_New( coral::PyCoral::AttributeListIterator,
                                                              coral::PyCoral::AttributeListIterator_Type() );
    PyObject* temp = Py_BuildValue((char*)"O", self);
    bool ok = (ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize AttributeList Iterator" );
      PyObject_Del(ob);
      return 0;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... )
  {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Error: AttributeListIterator not created" );
    return 0;
  }
}


PyObject*
AttributeList_attributeByName( PyObject* self, PyObject* args )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    if ( PyNumber_Check( args ) ) {
      PyObject* aLong = PyNumber_Long( args );
      int index = PyLong_AsLong(aLong);
      Py_DECREF( aLong );
      return AttributeList_attributeByIndex( self, index );
    }

    if ( PyString_Check( args ) ) {
      std::string value = std::string( PyString_AS_STRING( args ) );
      const coral::AttributeList& theList = *(py_this->object);
      coral::Attribute* theAttribute = const_cast<coral::Attribute*>( &(  theList[ value ] ) );
      if (! theAttribute ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error: the underlying C++ Attribute Object is NULL." );
        return 0;
      }

      coral::PyCoral::Attribute* ob = PyObject_New( coral::PyCoral::Attribute,
                                                    coral::PyCoral::Attribute_Type() );
      if ( ! ob )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Creating Attribute object." );
        return 0;
      }
      PyObject* c_object = PyCObject_FromVoidPtr( theAttribute, 0 );
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
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... )
  {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Invalid argument type specified" );
    return 0;
  }
  PyErr_SetString( coral::PyCoral::Exception(),
                   (char*)"Invalid Index (Neither String Nor Numeric) specified" );
  Py_RETURN_NONE;
}

PyObject*
AttributeList_attributeByIndex( PyObject* self, int index )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    const coral::AttributeList& theList = *(py_this->object);
    coral::Attribute* theAttribute = const_cast<coral::Attribute*>( &(  theList[ index ] ) );
    if (! theAttribute ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: the underlying C++ Attribute Object is NULL." );
      return 0;
    }
    coral::PyCoral::Attribute* ob = PyObject_New( coral::PyCoral::Attribute,
                                                  coral::PyCoral::Attribute_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating Attribute object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theAttribute, 0 );
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
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... )
  {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Invalid argument type specified" );
    return 0;
  }
}


int
AttributeList_length( PyObject* self )
{
  coral::PyCoral::AttributeList* py_this = (coral::PyCoral::AttributeList*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return -1;
  }
  try {
    return py_this->object->size();
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch( ... )
  {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Cannot retrieve length of the Attribute List" );
    return -1;
  }
}
