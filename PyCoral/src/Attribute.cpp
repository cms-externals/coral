#include "Attribute.h"
#include "CoralBase/Attribute.h"
#include "AttributeSpecification.h"
#include "CoralBase/AttributeSpecification.h"
#include "PyCoral/Date.h"
#include "CoralBase/Date.h"
#include "PyCoral/Blob.h"
#include "CoralBase/Blob.h"
#include "PyCoral/TimeStamp.h"
#include "CoralBase/TimeStamp.h"
#include "Exception.h"
#include <sstream>

// Forward declaration of the methods

static int Attribute_init( PyObject* self, PyObject* args , PyObject* /* kwds */);
static void Attribute_dealloc( PyObject* self );

static PyObject* Attribute_specification( PyObject* self);
static PyObject* Attribute_data( PyObject* self);
static PyObject* Attribute_size( PyObject* self);
static PyObject* Attribute_setData( PyObject* self, PyObject* args);
static PyObject* Attribute_shareData( PyObject* self, PyObject* args);
static PyObject* Attribute_str( PyObject* self );
static PyObject* Attribute_isNull( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::Attribute_Type()
{
  static PyMethodDef Attribute_Methods[] = {
    { (char*) "specification", (PyCFunction) Attribute_specification, METH_NOARGS,
      (char*) "Returns the specification of the attribute" },
    { (char*) "data", (PyCFunction) Attribute_data, METH_NOARGS,
      (char*) "Retrieves the data" },
    { (char*) "size", (PyCFunction) Attribute_size, METH_NOARGS,
      (char*) "Retrieves the size in bytes of the data" },
    { (char*) "setData", (PyCFunction) Attribute_setData, METH_O,
      (char*) "Sets the value from external source" },
    { (char*) "shareData", (PyCFunction) Attribute_shareData, METH_VARARGS,
      (char*) "Shares data with another attribute." },
    { (char*) "isNull", (PyCFunction) Attribute_isNull, METH_NOARGS,
      (char*) "Retrieves the NULL-ness of the variable." },
    {0, 0, 0, 0}
  };

  static char Attribute_doc[] = "A class defining Attribute.";

  static PyTypeObject Attribute_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.Attribute", /*tp_name*/
    sizeof(coral::PyCoral::Attribute), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    Attribute_dealloc, /*tp_dealloc*/
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
    Attribute_str, /*tp_str*/
    PyObject_GenericGetAttr, /*tp_getattro*/
    PyObject_GenericSetAttr, /*tp_setattro*/
    0, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    Attribute_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    Attribute_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    Attribute_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    Attribute_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &Attribute_Type;
}

int
Attribute_init( PyObject* self, PyObject* args , PyObject* /* kwds */)
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;

  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }

  py_this->object = 0;
  py_this->parent = 0;

  PyObject* c_object = 0;
  // Exception is raised by ParseTuple itself, on error, so no need to set ErrString here
  if ( !PyArg_ParseTuple( args, (char*)"OO",
                          &(py_this->parent),
                          &c_object ) ) return -1;
  py_this->object = static_cast<coral::Attribute*>
    ( PyCObject_AsVoidPtr( c_object ) );

  if ( py_this->parent ) Py_INCREF( py_this->parent );

  return 0;
}

void
Attribute_dealloc( PyObject* self )
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;
  py_this->object = 0;

  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }

  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject* Attribute_specification( PyObject* self)
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;

  if ( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    coral::AttributeSpecification* theData = const_cast<coral::AttributeSpecification*>
      (& ( py_this->object->specification() ) );
    coral::PyCoral::AttributeSpecification* ob = PyObject_New( coral::PyCoral::AttributeSpecification,
                                                               coral::PyCoral::AttributeSpecification_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating AttributeSpecification object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theData, 0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Initialising AttributeSpecification object." );
      PyObject_Del(ob);
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(), "Failed to retrieve the specification of an attribute" );
    return 0;
  }
}


PyObject*
Attribute_data( PyObject* self )
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;
  coral::Attribute* coralAttributeDataObject = py_this->object;

  if ( ! coralAttributeDataObject ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    if ( coralAttributeDataObject->isNull() ) {
      Py_RETURN_NONE;
    }

    const coral::AttributeSpecification& specification = coralAttributeDataObject->specification();
    const std::type_info& typeInfo = specification.type();

    if ( typeInfo == typeid(bool) ) {
      if ( coralAttributeDataObject->data<bool>() ) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    }
    else if ( typeInfo == typeid(char) ) {
      return Py_BuildValue( (char*) "c", coralAttributeDataObject->data<char>() );
    }
    else if ( typeInfo == typeid(unsigned char) ) {
      return Py_BuildValue( (char*) "B", coralAttributeDataObject->data<unsigned char>() );
    }
    else if ( typeInfo == typeid(int) ) {
      return Py_BuildValue( (char*) "i", coralAttributeDataObject->data<int>() );
    }
    else if ( typeInfo == typeid(unsigned int) ) {
      return Py_BuildValue( (char*) "k", coralAttributeDataObject->data<unsigned int>() );
    }
    else if ( typeInfo == typeid(short int) ) {
      return Py_BuildValue( (char*) "h", coralAttributeDataObject->data<short int>() );
    }
    else if ( typeInfo == typeid(unsigned short int) ) {
      return Py_BuildValue( (char*) "H", coralAttributeDataObject->data<unsigned short int>() );
    }
    else if ( typeInfo == typeid(long) ) {
      return Py_BuildValue( (char*) "l", coralAttributeDataObject->data<long>() );
    }
    else if ( typeInfo == typeid(unsigned long) ) {
      return Py_BuildValue( (char*) "k", coralAttributeDataObject->data<unsigned long>() );
    }
    else if ( typeInfo == typeid(long long) ) {
      return Py_BuildValue( (char*) "L", coralAttributeDataObject->data<long long>() );
    }
    else if ( typeInfo == typeid(unsigned long long) ) {
      return Py_BuildValue( (char*) "K", coralAttributeDataObject->data<unsigned long long>() );
    }
    else if ( typeInfo == typeid(float) ) {
      return Py_BuildValue( (char*) "f", coralAttributeDataObject->data<float>() );
    }
    else if ( typeInfo == typeid(double) ) {
      return Py_BuildValue( (char*) "d", coralAttributeDataObject->data<double>() );
    }
    else if ( typeInfo == typeid(long double) ) {
      return Py_BuildValue( (char*) "d", static_cast<double>( coralAttributeDataObject->data<long double>() ) );
      // TEMPORARY CASTING UNTIL PYTHON SUPPORTS LONG DOUBLE!!!
    }
    else if ( typeInfo == typeid(std::string) ) {
      std::string stringData = coralAttributeDataObject->data<std::string>();
      return Py_BuildValue( (char*) "s", stringData.c_str() );
    }
    else if ( typeInfo == typeid(coral::Date) ) {
      const coral::Date& refData = coralAttributeDataObject->data<coral::Date>();
      coral::Date* theData = const_cast<coral::Date*>(&refData);
      coral::PyCoral::Date* ob = PyObject_New( coral::PyCoral::Date, coral::PyCoral::Date_Type() );
      if ( ! ob )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Creating Date object." );
        return 0;
      }
      PyObject* c_object = PyCObject_FromVoidPtr( theData, 0 );
      PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
      bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
      Py_DECREF(temp);
      Py_DECREF( c_object );
      if (ok)
        return (PyObject*) ob;
      else{
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Initialising Date object." );
        PyObject_Del(ob);
        return 0;
      }
    }
    else if ( typeInfo == typeid(coral::TimeStamp) ) {
      const coral::TimeStamp& refData = coralAttributeDataObject->data<coral::TimeStamp>();
      coral::TimeStamp* theData = const_cast<coral::TimeStamp*>(&refData);
      coral::PyCoral::TimeStamp* ob = PyObject_New( coral::PyCoral::TimeStamp, coral::PyCoral::TimeStamp_Type() );
      if ( ! ob )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Creating TimeStamp object." );
        return 0;
      }
      PyObject* c_object = PyCObject_FromVoidPtr( theData, 0 );
      PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
      bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
      Py_DECREF(temp);
      Py_DECREF( c_object );
      if (ok)
        return (PyObject*) ob;
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Initialising TimeStamp object." );
        PyObject_Del(ob);
        return 0;
      }
    }
    //   else if ( typeInfo == typeid(seal::IntBits<64>::SLeast) ) {
    //    return Py_BuildValue( (char*) "L", coralAttributeDataObject->data<seal::IntBits<64>::SLeast>() );
    //   }
    //   else if ( typeInfo == typeid(seal::IntBits<64>::ULeast) ) {
    //    return Py_BuildValue( (char*) "K", coralAttributeDataObject->data<seal::IntBits<64>::ULeast>() );
    //   }
    else if ( typeInfo == typeid(coral::Blob) ) {
      const coral::Blob& refData = coralAttributeDataObject->data<coral::Blob>();
      coral::Blob* theData = const_cast<coral::Blob*>(&refData);
      coral::PyCoral::Blob* ob = PyObject_New( coral::PyCoral::Blob, coral::PyCoral::Blob_Type() );
      if ( ! ob )
      {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Creating Blob object." );
        return 0;
      }
      PyObject* c_object = PyCObject_FromVoidPtr( theData, 0 );
      PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
      bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
      Py_DECREF(temp);
      Py_DECREF( c_object );
      if (ok)
        return (PyObject*) ob;
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*) "Error in Initialising Blob object." );
        PyObject_Del(ob);
        return 0;
      }
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     "Could not retrieve the data from an attribute" );
    return 0;
  }
  PyErr_SetString( coral::PyCoral::Exception(),
                   "TypeInfo not Supported" );
  return 0;
}

PyObject*
Attribute_setData( PyObject* self, PyObject* args )
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;
  coral::Attribute* coralAttributeDataObject = py_this->object;

  if ( ! coralAttributeDataObject ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    if ( args == Py_None  ) {
      coralAttributeDataObject->setNull( true );
      return Py_BuildValue( (char*) "" );
    }

    coralAttributeDataObject->setNull( false );
    const std::type_info& typeInfo = coralAttributeDataObject->specification().type();

    if ( typeInfo == typeid(bool) ) {
      coralAttributeDataObject->setValue<bool>( ( PyObject_IsTrue( args ) ? true : false ) );
    }
    else if ( typeInfo == typeid(char) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<char>( (char)PyLong_AsLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<char>( (char)PyInt_AsLong(args) );
      }
      else if ( PyString_Check(args) && (PyString_GET_SIZE( args ) == 1) ) {
        coralAttributeDataObject->setValue<char>( *( PyString_AS_STRING( args ) ) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a char!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(unsigned char) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned char>( (unsigned char)PyLong_AsUnsignedLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned char>( (unsigned char)PyInt_AsUnsignedLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an unsigned char!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(int) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<int>( PyLong_AsLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<int>( PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(unsigned int) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned int>( PyLong_AsUnsignedLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned int>( PyInt_AsUnsignedLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an unsigned integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(short int) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<short>( (short)PyLong_AsLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<short>( (short)PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a short integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(unsigned short) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned short>( (unsigned short)PyLong_AsUnsignedLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned short>( (unsigned short)PyInt_AsUnsignedLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an unsigned short integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(long) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<long>( PyLong_AsLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<long>( PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a Long integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(unsigned long) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned long>( PyLong_AsUnsignedLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned long>( PyInt_AsUnsignedLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an unsigned long integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(long long) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<long long>( PyLong_AsLongLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<long long>( PyInt_AsUnsignedLongLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a long long integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(unsigned long long) ) {
      if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned long long>( PyLong_AsUnsignedLongLong(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<unsigned long long>( PyInt_AsUnsignedLongLongMask(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not an unsigned long long integer!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(float) ) {
      if ( PyFloat_Check( args ) ) {
        coralAttributeDataObject->setValue<float>( (float)PyFloat_AS_DOUBLE(args) );
      }
      else if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<float>( (float)PyLong_AsDouble(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<float>( (float)PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a float!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(double) ) {
      if ( PyFloat_Check( args ) ) {
        coralAttributeDataObject->setValue<double>( PyFloat_AS_DOUBLE(args) );
      }
      else if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<double>( PyLong_AsDouble(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<double>( PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not double!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(long double) ) {
      if ( PyFloat_Check( args ) ) {
        coralAttributeDataObject->setValue<long double>( PyFloat_AS_DOUBLE(args) );
      }
      else if ( PyLong_Check( args ) ) {
        coralAttributeDataObject->setValue<long double>( PyLong_AsDouble(args) );
      }
      else if ( PyInt_Check( args ) ) {
        coralAttributeDataObject->setValue<long double>( PyInt_AS_LONG(args) );
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not Long Double!" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(std::string) ) {
      if ( ! PyString_Check( args ) ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not a string" );
        return 0;
      }
      coralAttributeDataObject->setValue<std::string>( PyString_AS_STRING( args ) );
    }
    else if ( typeInfo == typeid(coral::Date) ) {
      if ( args->ob_type != coral::PyCoral::Date_Type() ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not date type" );
        return 0;
      }
      coral::PyCoral::Date* pyData = (coral::PyCoral::Date*) args;
      coral::Date* theData = pyData->object;
      if ( theData ) {
        py_this->object->setValue<coral::Date>( *theData);
      }
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Could not set the data of a Date type" );
        return 0;
      }
    }
    else if ( typeInfo == typeid(coral::TimeStamp) ) {
      if ( args->ob_type != coral::PyCoral::TimeStamp_Type() ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not time stamp type" );
        return 0;
      }
      coral::PyCoral::TimeStamp* pydata = (coral::PyCoral::TimeStamp*) args;
      coral::TimeStamp* inputData = pydata->object;
      if ( inputData )
        coralAttributeDataObject->setValue<coral::TimeStamp>( *inputData);
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Could not set the data of an timestamp type" );
        return 0;
      }
    }
    //   else if ( typeInfo == typeid(seal::IntBits<64>::SLeast) ) {
    //    if ( PyLong_Check( args ) ) {
    //     coralAttributeDataObject->setValue<seal::IntBits<64>::SLeast>( PyLong_AsLongLong(args) );
    //    }
    //    else if ( PyInt_Check( args ) ) {
    //     coralAttributeDataObject->setValue<seal::IntBits<64>::SLeast>( PyInt_AS_LONG(args) );
    //    }
    //    else {
    //     PyErr_SetString( coral::PyCoral::Exception(),
    //                      (char*)"Argument is not a 64 bit Long Long integer!" );
    //     return 0;
    //    }
    //   }
    //   else if ( typeInfo == typeid(seal::IntBits<64>::ULeast) ) {
    //    if ( PyLong_Check( args ) ) {
    //     coralAttributeDataObject->setValue<seal::IntBits<64>::ULeast>( PyLong_AsUnsignedLongLong(args) );
    //    }
    //    else if ( PyInt_Check( args ) ) {
    //     coralAttributeDataObject->setValue<seal::IntBits<64>::ULeast>( PyInt_AsUnsignedLongLongMask(args) );
    //    }
    //    else {
    //     PyErr_SetString( coral::PyCoral::Exception(),
    //                      (char*)"Argument is not a 64 bit unsigned Long Long integer!" );
    //     return 0;
    //    }
    //  }
    else if ( typeInfo == typeid(coral::Blob) ) {
      if ( args->ob_type != coral::PyCoral::Blob_Type() ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Argument is not blob type" );
        return 0;
      }
      coral::PyCoral::Blob* pydata = (coral::PyCoral::Blob*) args;
      coral::Blob* inputData = pydata->object;
      if ( inputData )
        coralAttributeDataObject->setValue<coral::Blob>( *inputData);
      else {
        PyErr_SetString( coral::PyCoral::Exception(),
                         (char*)"Could not set the data of an empty blob type" );
        return 0;
      }
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not set the data of an attribute" );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
Attribute_size( PyObject* self )
{
  coral::PyCoral::Attribute* py_this = ( coral::PyCoral::Attribute* )self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    coral::Attribute& coralAttribute = *( py_this->object );
    return Py_BuildValue( (char *) "k", coralAttribute.size() );
  }
  catch(  std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retrieve the data of an attribute" );
    return 0;
  }
}



PyObject*
Attribute_str( PyObject* self )
{
  coral::PyCoral::Attribute* py_this = ( coral::PyCoral::Attribute* )self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  std::ostringstream os;
  coral::Attribute& coralAttribute = *( py_this->object );
  coralAttribute.toOutputStream( os );
  std::string resultStr = os.str();
  return Py_BuildValue( (char *) "s", resultStr.c_str() );
}

PyObject*
Attribute_shareData( PyObject* self, PyObject* sourceAttribute)
{
  coral::PyCoral::Attribute* py_this = ( coral::PyCoral::Attribute* )self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    coral::PyCoral::Attribute* srcAttribute = 0;
    if( ! PyArg_ParseTuple( sourceAttribute,(char*) "O!", coral::PyCoral::Attribute_Type(), &srcAttribute ) ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Bad Argument Type" );
      return 0;
    }
    //Py_INCREF( (PyObject*) srcAttribute );
    if ( srcAttribute->object )
      py_this->object->shareData( *(srcAttribute->object) );
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Can not share the data with an empty attribute" );
      return 0;
    }
  }
  catch( std::exception& e) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error has occured while sharing the attribute. " );
    return 0;
  }

  Py_RETURN_NONE;
}


PyObject*
Attribute_isNull( PyObject* self )
{
  coral::PyCoral::Attribute* py_this = (coral::PyCoral::Attribute*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isNull() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving is Null for the Attribute" );
    return 0;
  }
}
