#include "Exception.h"
#include "IMonitoringReporter.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/IMonitoring.h"
#include <sstream>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyBytes_Check
    #define PyString_AS_STRING PyBytes_AS_STRING
    #define PyString_AsString PyBytes_AsString
    #define PyString_GET_SIZE PyBytes_GET_SIZE
    #define PyString_FromString PyBytes_FromString
#endif
// Forward declaration of the methods
static int IMonitoringReporter_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IMonitoringReporter_dealloc( PyObject* self );
static PyObject* IMonitoringReporter_str( PyObject* self, PyObject* args );

static PyObject* IMonitoringReporter_monitoredDataSources( PyObject* self );
static PyObject* IMonitoringReporter_report( PyObject* self, PyObject* args );
static PyObject* IMonitoringReporter_reportToOutputStream( PyObject* self, PyObject* args );
/// Note: reportToOutputStream  OR str will only output the result to the standard sys.stdout stream ////

// Type definition
PyTypeObject*
coral::PyCoral::IMonitoringReporter_Type()
{
  static PyMethodDef IMonitoringReporter_Methods[] = {
    { (char*) "monitoredDataSources", (PyCFunction)(void *) IMonitoringReporter_monitoredDataSources, METH_NOARGS,
      (char*) "Return the set of currently monitored data sources." },
    { (char*) "report", (PyCFunction)(void *) IMonitoringReporter_report, METH_VARARGS,
      (char*) "Reports the events for all data sources being monitored \
               * @param level      The OR-ed selection of even types to be reported\
               OR Reports the events for a given data source name of a given monitoring level\
               * @param contextKey The session ID for which to make the report\
               * @param level      The OR-ed selection of even types to be reported.  " },
    { (char*) "reportToOutputStream", (PyCFunction)(void *) IMonitoringReporter_reportToOutputStream, METH_VARARGS,
      (char*) "Reports the events for a given data source name to the specified output stream\
               * @param contextKey The session ID for which to make the report\
               * @param os         The output stream\
               * @param level      The OR-ed selection of even types to be reported ." },
    { (char*) "str", (PyCFunction)(void *) IMonitoringReporter_str, METH_VARARGS,
      (char*) "Reports the events for a given data source name to the specified output stream\
               * @param contextKey The session ID for which to make the report\
               * @param os         The output stream\
               * @param level      The OR-ed selection of even types to be reported ." },
    {0, 0, 0, 0}
  };

  static char IMonitoringReporter_doc[] = "User-level interface for the client side monitoring system. If any of the calls fails a MonitoringException is thrown.";

  static PyTypeObject IMonitoringReporter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IMonitoringReporter", // tp_name
    sizeof(coral::PyCoral::IMonitoringReporter), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IMonitoringReporter_dealloc, // tp_dealloc
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
    IMonitoringReporter_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IMonitoringReporter_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IMonitoringReporter_init, // tp_init
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
    IMonitoringReporter_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IMonitoringReporter_Type;
}

int
IMonitoringReporter_init( PyObject* self, PyObject* args, PyObject* /*kwds*/ )
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
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
  py_this->object = static_cast<coral::IMonitoringReporter*>( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IMonitoringReporter_dealloc( PyObject* self )
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IMonitoringReporter_monitoredDataSources( PyObject* self )
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    std::set<std::string> result = py_this->object->monitoredDataSources();
    PyObject* py_tuple = PyTuple_New(result.size());
    if (!py_tuple) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Unable to create Tuple Object");
      return 0;
    }
    int i=0;
    for( std::set<std::string>::const_iterator theIterator = result.begin();
         theIterator != result.end(); theIterator++ ) {
      PyObject* str = PyString_FromString(theIterator->c_str());
      PyTuple_SET_ITEM(py_tuple,i++,str);
    }
    return py_tuple;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving all monitored data sources." );
    return 0;
  }
}

PyObject*
IMonitoringReporter_report( PyObject* self, PyObject* args)
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* contextKey = 0;
    coral::monitor::Level level = coral::monitor::Default;
    int numberOfArguments = PyTuple_GET_SIZE( args );
    if ( numberOfArguments == 0 ) {
      py_this->object->report();
    }
    if ( numberOfArguments == 2 ) {
      if( ! PyArg_ParseTuple( args, (char*) "si", &contextKey, &level ) ) return 0;
      py_this->object->report(std::string(contextKey), level);
    }
    if ( numberOfArguments == 1 ) {
      if (PyString_Check(PyTuple_GET_ITEM(args,0))) {
        if( ! PyArg_ParseTuple( args, (char*) "s", &contextKey ) ) return 0;
        py_this->object->report( std::string(contextKey) );
      }
      else{
        if (PyNumber_Check(PyTuple_GET_ITEM(args,0))) {
          if( ! PyArg_ParseTuple( args, (char*) "i", &level ) ) return 0;
          py_this->object->report( level );
        }
      }
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving report" );
    return 0;
  }
  Py_RETURN_NONE;
}


PyObject*
IMonitoringReporter_reportToOutputStream( PyObject* self, PyObject* args)
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* contextKey = 0;
    std::ostringstream os;
    char* os1 = 0;
    coral::monitor::Level level = coral::monitor::Default;
    if ( ! PyArg_ParseTuple(args, (char*) "ss|i", &contextKey,&os1,&level) ) return 0;
    py_this->object->reportToOutputStream(std::string(contextKey), os, level );
    std::string result = os.str();
    return Py_BuildValue( (char*) "s", result.c_str() );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when reporting to output stream" );
    return 0;
  }
}


PyObject*
IMonitoringReporter_str( PyObject* self, PyObject* args)
{
  coral::PyCoral::IMonitoringReporter* py_this = (coral::PyCoral::IMonitoringReporter*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* contextKey = 0;
    std::ostringstream os;
    char* os1 = 0;
    coral::monitor::Level level = coral::monitor::Default;
    if ( ! PyArg_ParseTuple(args, (char*) "ss|i", &contextKey,&os1,&level) ) return 0;
    py_this->object->reportToOutputStream(std::string(contextKey), os, level );
    std::string result = os.str();
    return Py_BuildValue( (char*) "s", result.c_str() );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when reporting to output stream" );
    return 0;
  }
}
