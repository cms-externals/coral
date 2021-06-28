#include "Exception.h"
#include "MessageStream.h"

#include "CoralBase/MessageStream.h"

static int MessageStream_init( PyObject* self, PyObject* args, PyObject* kwds );
static void MessageStream_dealloc( PyObject* self );

static PyObject* MessageStream_setMsgVerbosity( PyObject* self, PyObject* args );
// Type definition
PyTypeObject*
coral::PyCoral::MessageStream_Type()
{
  static PyMethodDef MessageStream_Methods[] = {
    { (char*) "setMsgVerbosity", (PyCFunction)(void *) MessageStream_setMsgVerbosity, METH_VARARGS,
      (char*) "Sets the Verbosity level of the MessageStream \
               * @param level      The level" },
    {0, 0, 0, 0}
  };

  static char MessageStream_doc[] = "A class defining a stream to handle messages.";

  static PyTypeObject MessageStream_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.MessageStream", // tp_name
    sizeof(coral::PyCoral::MessageStream), // tp_basicsize
    0, // tp_itemsize
       //  methods
    MessageStream_dealloc, // tp_dealloc
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
    MessageStream_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    MessageStream_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    MessageStream_init, // tp_init
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
    MessageStream_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &MessageStream_Type;

}

int
MessageStream_init( PyObject* self, PyObject* args, PyObject* kwds )
{
  //cast the self pointer to message stream
  coral::PyCoral::MessageStream* py_this = (coral::PyCoral::MessageStream*)self;

  if ( py_this == NULL ) {
    PyErr_SetString( coral::PyCoral::Exception(), (char*) "Error: Self is NULL" );
    return -1;
  }

  py_this->object = NULL;
  py_this->parent = NULL;

  int numberOfArguments = PyTuple_GET_SIZE( args );
  int numberOfKeyWords = ( kwds ) ? PyTuple_GET_SIZE( kwds ) : 0;

  if ( (numberOfArguments == 1) && (numberOfKeyWords == 0) ) {
    //get the source parameter from python
    char* source = NULL;
    if( ! PyArg_ParseTuple( args, (char*) "s", &source ) ) {
      PyErr_SetString( coral::PyCoral::Exception(), (char*)"Can't convert parameter to string" );
      return -1;
    }
    py_this->object = new coral::MessageStream(source);

  }else{
    PyErr_SetString( coral::PyCoral::Exception(), (char*)"Please specify a source parameter in the constructor" );
    return -1;
  }
  return 0;
}

void
MessageStream_dealloc( PyObject* self )
{
  //cast the self pointer to message stream
  coral::PyCoral::MessageStream* py_this = (coral::PyCoral::MessageStream*)self;
  if(py_this->object != NULL) delete py_this->object;
  if ( py_this->parent != NULL ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = NULL;
  self->ob_type->tp_free( self );
}

PyObject*
MessageStream_setMsgVerbosity( PyObject* self, PyObject* args )
{
  //cast the self pointer to message stream
  coral::PyCoral::MessageStream* py_this = (coral::PyCoral::MessageStream*)self;
  if(py_this->object == NULL) {
    PyErr_SetString( coral::PyCoral::Exception(), (char*)"Error: No Valid C++ Object" );
    return NULL;
  }

  int numberOfArguments = PyTuple_GET_SIZE( args );

  if(numberOfArguments == 1) {

    coral::MsgLevel lvl = coral::Nil;

    if( ! PyArg_ParseTuple( args, (char*) "i", &lvl ) ) {
      PyErr_SetString( coral::PyCoral::Exception(), (char*)"Not a valid Message Type" );
      return NULL;
    }
    py_this->object->setMsgVerbosity(lvl);
  }else{
    PyErr_SetString( coral::PyCoral::Exception(), (char*)"Please specify a parameter" );
    Py_RETURN_NONE;
  }
  Py_RETURN_NONE;
}
