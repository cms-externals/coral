#include "PyCoral/Blob.h"
#include "CoralBase/Blob.h"
#include "Exception.h"
#include <sstream>

// Forward declaration of the methods
static int Blob_init( PyObject* self, PyObject* args, PyObject* /* kwds */ );
static void Blob_dealloc( PyObject* self );

#if PY_MAJOR_VERSION >= 3
static int  Blob_getbuffer(PyObject *obj, Py_buffer *view, int flags);
static void  Blob_releasebuffer(PyObject *obj, Py_buffer *view);
#else
static long int  Blob_getReadBuffer( PyObject *self, long int idx, void **ptrptr );
static long int  Blob_getCharBuffer( PyObject *self, long int idx, void **ptrptr );
static long int Blob_getWriteBuffer( PyObject *self, long int idx, void **ptrptr );
static long int Blob_getSegCount( PyObject *self, long int* lenp );
#endif
static PyObject* Blob_size( PyObject* self );
static PyObject* Blob_extend( PyObject* self, PyObject* args );
static PyObject* Blob_resize( PyObject* self, PyObject* args );
static PyObject* Blob_write( PyObject* self, PyObject* args );
static PyObject* Blob_read( PyObject* self, PyObject* args );
static PyObject* Blob_readline( PyObject* self );
static PyObject* Blob_append( PyObject* self, PyObject* args );

// Type definition
PyTypeObject*
coral::PyCoral::Blob_Type()
{
  static PyMethodDef Blob_Methods[] = {
    { (char*) "size", (PyCFunction)(void *) Blob_size, METH_NOARGS,
      (char*) "Returns the size of the BLOB. " },
    { (char*) "extend", (PyCFunction)(void *) Blob_extend, METH_O,
      (char*) "Extends the BLOB by additional size in bytes." },
    { (char*) "resize", (PyCFunction)(void *) Blob_resize , METH_O,
      (char*) "Resize a BLOB to size in bytes." },
    { (char*) "write", (PyCFunction)(void *) Blob_write , METH_VARARGS,
      (char*) "Writes a string or buffer into the BLOB, after extending it accordingly" },
    { (char*) "read", (PyCFunction)(void *) Blob_read , METH_O,
      (char*) "Read a string or buffer from the BLOB" },
    { (char*) "readline", (PyCFunction)(void *) Blob_readline , METH_NOARGS,
      (char*) "Reads the entire BLOB as one string" },
    { (char*) "append", (PyCFunction)(void *) Blob_append , METH_VARARGS,
      (char*) "Appends the data of another BLOB." },
    {0, 0, 0, 0}
  };

  static char Blob_doc[] = "A class wrapping a coral::Blob.It implements the Python buffer protocol.";
  static PyBufferProcs Blob_BufferMethods = {

// TODO: implement the Python3 buffer protocol.
#if PY_VERSION_HEX >= 0x03000000
    (getbufferproc) Blob_getbuffer, //  bf_getbuffer
    (releasebufferproc) Blob_releasebuffer //  bf_releasebuffer

    // caution: python interface change between 2.4->2.5
    // old typedefs (get...) still exist but are broken from 2.5 on 64bit amd & osx
#elif PY_VERSION_HEX >= 0x02050000
    (readbufferproc) Blob_getReadBuffer, //  bf_getreadbuffer
    (writebufferproc) Blob_getWriteBuffer, //  bf_getwritebuffer
    (segcountproc) Blob_getSegCount, //  bf_getsegcount
    (charbufferproc) Blob_getCharBuffer //  bf_getcharbuffer
  #if PY_VERSION_HEX >= 0x02060000
    ,(getbufferproc) 0, //  bf_getbuffer
    (releasebufferproc) 0 //  bf_releasebuffer
  #endif
#else
    (getreadbufferproc) Blob_getReadBuffer, //  bf_getreadbuffer
    (getwritebufferproc) Blob_getWriteBuffer, //  bf_getwritebuffer
    (getsegcountproc) Blob_getSegCount, //  bf_getsegcount
    (getcharbufferproc) Blob_getCharBuffer //  bf_getcharbuffer
#endif
  };

  static PyTypeObject Blob_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.Blob", // tp_name
    sizeof(coral::PyCoral::Blob), // tp_basicsize
    0, // tp_itemsize
       //  methods
    Blob_dealloc, // tp_dealloc
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
    &Blob_BufferMethods, // tp_as_buffer
    Py_TPFLAGS_DEFAULT, // tp_flags
    Blob_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    Blob_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    Blob_init, // tp_init
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
    Blob_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &Blob_Type;
}
#if PY_MAJOR_VERSION >= 3
static int
Blob_getbuffer(PyObject *obj, Py_buffer *view, int flags)
{
    coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) obj;
    void *ptr;
    if (view == NULL) {
        PyErr_SetString(PyExc_BufferError,
            "bytearray_getbuffer: view==NULL argument is obsolete");
        return -1;
    }

    ptr = (void *) py_this->object->startingAddress();
    /* cannot fail if view != NULL and readonly == 0 */
    try {
        PyBuffer_FillInfo(view, obj, ptr, py_this->object->size(), 0, flags);
    } 
    catch ( ... ) {
        PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not get the buffer of the BLOB" );
        return -1;
    }
    py_this->ob_exports++;
    return 0;
}
static void
Blob_releasebuffer(PyObject *obj, Py_buffer *view)
{
    coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) obj;
    py_this->ob_exports--;
    //release memory if it reaches 0?
}

#else

long int
Blob_getReadBuffer(PyObject* self, long int idx, void** pp)
{
  coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) self;
  if (idx != 0 )
  {
    PyErr_SetString(PyExc_SystemError,
                    (char*)"accessing non-existent buffer segment");
    return -1;
  }
  int returnValue = 0;
  if ( ! py_this->object ) {
    *pp = 0;
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    returnValue = 0;
  }
  else {
    try {
      *pp = py_this->object->startingAddress();
      returnValue = py_this->object->size();
    }
    catch( std::exception& e ) {
      PyErr_SetString( coral::PyCoral::Exception(), e.what() );
      returnValue = -1;
    }
    catch( ... ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Error retrieving the starting address of the BLOB" );
      returnValue = -1;
    }
  }
  return returnValue;
}

long int
Blob_getWriteBuffer(PyObject* self, long int idx, void** pp)
{
  return Blob_getReadBuffer(self, idx, pp);
}


long int
Blob_getCharBuffer(PyObject* self, long int idx, void** pp)
{
  return Blob_getReadBuffer(self, idx, pp);
}


long int
Blob_getSegCount(PyObject* self, long int* lenp)
{
  coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) self;

  if ( ! py_this->object ) {
    if ( lenp ) *lenp = 0;
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  else {
    try {
      if ( lenp ) *lenp = py_this->object->size();
      return 1;
    }
    catch( std::exception& e ) {
      PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    }
    catch (...) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not determine the size of the Blob" );
    }
    if ( lenp ) *lenp = 0;
    return 0;
  }
}
#endif


int
Blob_init( PyObject* self, PyObject* args, PyObject* /* kwds */ )
{
  coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );

    return -1;
  }
  py_this->object = 0;
  py_this->currentPosition = 0;
  py_this->parent = 0;

  try{
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 )
    { // If no arguments are passed it creates an empty BLOB
      py_this->object = new coral::Blob();
      return 0;
    }
    else {
      if ( numberOfArguments == 2 ) {
        PyObject* c_object = 0;
        if ( !PyArg_ParseTuple( args, (char*)"OO",
                                &(py_this->parent),
                                &c_object ) ) return -1;
        py_this->object = static_cast<coral::Blob*>
          ( PyCapsule_GetPointer( c_object , "name") );
        if ( py_this->parent ) Py_INCREF( py_this->parent );
        return 0;
      }
      long sizeInBytes = 0;
      if ( ! PyArg_ParseTuple( args, (char*) "k", &sizeInBytes ) ) return -1;
      py_this->object = new coral::Blob( sizeInBytes );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not create the BLOB" );
    return -1;
  }
}


void
Blob_dealloc( PyObject* self )
{
  coral::PyCoral::Blob* py_this = (coral::PyCoral::Blob*) self;
  if ( py_this->object && ! py_this->parent ) {
    delete py_this->object;
    py_this->object = 0;
  }
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
    py_this->parent = 0;
  }
  py_this->currentPosition=0;
  self->ob_type->tp_free( self );
}


PyObject*
Blob_size( PyObject* self )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    long blobSize = py_this->object->size();
    return Py_BuildValue( (char*) "k", blobSize);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retrieve the size." );
    return 0;
  }
}


PyObject*
Blob_extend( PyObject* self, PyObject* args )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    if (PyLong_Check( args ) ) {
      py_this->object->extend( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
           else if ( PyInt_Check( args ) ) {
      py_this->object->extend( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not a Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not extend the BLOB." );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
Blob_resize( PyObject* self, PyObject* args )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try{
    if (PyLong_Check( args ) ) {
      py_this->object->resize( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->resize( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not a Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not resize the BLOB." );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
Blob_write( PyObject* self, PyObject* args )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    const char* data = 0;
    int sizeOfData = 0;
    if ( ! PyArg_ParseTuple( args, (char*) "s#", &data, &sizeOfData ) ) return 0;
    long currentSize = py_this->object->size();
    py_this->object->extend( sizeOfData );
    void* addressToWrite = static_cast<char*>( py_this->object->startingAddress() ) + currentSize;
    ::memcpy( addressToWrite, data, sizeOfData );
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not write into the BLOB." );
    return 0;
  }
}

PyObject*
Blob_read( PyObject* self, PyObject* args )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( args == Py_None  ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is None Type." );
      return 0;
    }
    long sizeOfData = 0;
    if ( PyLong_Check( args )) {
      sizeOfData = PyLong_AsLong( args );

    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
      #ifdef __clang__
        } else if PyInt_CheckExact( args ) {
      #else
        } else if ( PyInt_CheckExact( args ) ) {  
      #endif
      sizeOfData = PyInt_AS_LONG( args );
    #endif
        }
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an Integer or long type." );
      return 0;
    }
    long blobSize = py_this->object->size();
    if ( sizeOfData > blobSize ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Error: Request To read bytes larger than BLOB size." );
      return 0;
    }
    void* addressToReadFrom = static_cast<char*>( py_this->object->startingAddress() )
      + py_this->currentPosition;
    py_this->currentPosition = py_this->currentPosition + sizeOfData;
    return Py_BuildValue( (char*)"s#",(char*)addressToReadFrom, sizeOfData);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not read from the BLOB." );
    return 0;
  }
}


PyObject*
Blob_readline( PyObject* self )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob*)self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    long blobSize = py_this->object->size();
    void* addressToReadFrom = static_cast<char*>( py_this->object->startingAddress() );
    return Py_BuildValue( (char*)"s#",(char*)addressToReadFrom,blobSize);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not read from the BLOB." );
    return 0;
  }
}


PyObject*
Blob_append( PyObject* self, PyObject* srcBlob )
{
  coral::PyCoral::Blob* py_this = ( coral::PyCoral::Blob* )self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  coral::PyCoral::Blob* rhs = 0;
  try{
    if ( ! PyArg_ParseTuple( srcBlob, (char*) "O!", coral::PyCoral::Blob_Type(), &rhs ) ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Bad blob object in argument" );
      return 0;
    }
    // Py_INCREF( (PyObject*) rhs );
    if( ! rhs->object ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: No Valid C++ Object" );
      return 0;
    }
    *(py_this->object) += *(rhs->object);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not append the BLOB." );
    return 0;
  }
}
