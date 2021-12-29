#include "ConnectionService.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/ISessionProxy.h"
#include "IConnectionServiceConfiguration.h"
#include "IWebCacheControl.h"
#include "IMonitoringReporter.h"
#include "ISessionProxy.h"
#include "Exception.h"
#include <exception>

#include "RelationalAccess/ConnectionService.h"

// Forward declaration of the methods
static int ConnectionService_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ConnectionService_dealloc( PyObject* self );
static PyObject* ConnectionService_purgeConnectionPool( PyObject* self );
static PyObject* ConnectionService_configuration( PyObject* self );
static PyObject* ConnectionService_connect( PyObject* self, PyObject* args, PyObject* kwds );
static PyObject* ConnectionService_webCacheControl( PyObject* self );
static PyObject* ConnectionService_monitoringReporter( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::ConnectionService_Type()
{
  static PyMethodDef ConnectionService_Methods[] = {
    { (char*) "purgeConnectionPool", (PyCFunction)(void *) ConnectionService_purgeConnectionPool, METH_NOARGS,
      (char*) "Purges the idle connections from the pool." },
    { (char*) "configuration", (PyCFunction)(void *) ConnectionService_configuration, METH_NOARGS,
      (char*) "Returns the configuration object for the service." },
    { (char*) "connect", (PyCFunction)(void *) ConnectionService_connect, METH_VARARGS | METH_KEYWORDS,
      (char*) "Returns a session proxy object for the specified connectionName, role and accessMode." },
    { (char*) "webCacheControl", (PyCFunction)(void *) ConnectionService_webCacheControl, METH_NOARGS,
      (char*) "Returns the object which controls the web cache." },
    { (char*) "monitoringReporter", (PyCFunction)(void *) ConnectionService_monitoringReporter, METH_NOARGS,
      (char*) "Returns the monitoring reporter." },
    {0, 0, 0, 0}
  };

  static char ConnectionService_doc[] = "A Connection Service class.\nIt loads the CORAL/Services/ConnectionService component into the parent or local context.\nIt is responsible for providing Session objects to the user.";

  static PyTypeObject ConnectionService_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.ConnectionService", // tp_name
    sizeof(coral::PyCoral::ConnectionService), // tp_basicsize
    0, // tp_itemsize
       //  methods
    ConnectionService_dealloc, // tp_dealloc
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
    ConnectionService_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    ConnectionService_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    ConnectionService_init, // tp_init
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
    ConnectionService_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &ConnectionService_Type;
}


/*
namespace coral {
  namespace PyCoral {
    static coral::IConnectionService* loadConnectionService( seal::Context* context );
  }
}
*///


int
ConnectionService_init( PyObject* self, PyObject* /*args*/, PyObject* /* kwds */ )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;

  // Check the length of the argument list
  //int numberOfArguments = PyTuple_GET_SIZE( args );

  /*
  if ( numberOfArguments == 0 ) { // Create a local context.
    py_this->context = PyObject_New( coral::PyCoral::Context, coral::PyCoral::Context_Type() );
    py_this->context->ob_type->tp_init( (PyObject*) py_this->context, 0, 0 );

    // Setting the verbosity level
    PyObject* ret = PyObject_CallMethod( (PyObject*) py_this->context,
                                         (char*) "setVerbosityLevel",
                                         (char*) "s", (char*) "INFO" );
    if ( !ret ){
      Py_DECREF( py_this->context );
      py_this->context = 0;
      Py_DECREF( ret );
      return -1;
    }
    Py_DECREF( ret );
  }
  else { // Retrieve the context from the argument list
    if( ! PyArg_ParseTuple( args, (char*) "O!", coral::PyCoral::Context_Type(), &( py_this->context ) ) ) {
      return -1;
    }
    Py_INCREF( py_this->context );
  }
  *///

  // Load the Connection Service into the context

  //  std::unique_ptr<coral::ConnectionService> connectionService( new coral::ConnectionService() );

  py_this->object = new coral::ConnectionService();

  /*
  try {
    py_this->object = coral::PyCoral::loadConnectionService( py_this->context->object );
    if ( ! py_this->object ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Could not load the Connection Service!" );
      Py_DECREF( py_this->context );
      py_this->context = 0;
      return -1;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    Py_DECREF( py_this->context );
    py_this->context = 0;
    return -1;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Could not load the Connection Service!!!" );
    Py_DECREF( py_this->context );
    py_this->context = 0;
    return -1;
  }
  *///

  return 0;
}

void
ConnectionService_dealloc( PyObject* self )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;
  delete py_this->object; // fix memory leak bug #100573
  py_this->object = 0;
  //  if ( py_this->context ) Py_DECREF( py_this->context );
  self->ob_type->tp_free( self );
}


/*
coral::IConnectionService*
coral::PyCoral::loadConnectionService( seal::Context* context )
{


  std::vector< seal::IHandle<coral::IConnectionService> > v_svc;
  context->query( v_svc );
  bool ok = true;
  if ( v_svc.empty() ) {
    ok = false;
    seal::Handle<seal::ComponentLoader> loader = new seal::ComponentLoader( context );
    loader->load( "CORAL/Services/ConnectionService" );
    context->query( v_svc );
    if ( v_svc.empty() ) {
      return 0;
    }
  }

  return &( *( v_svc.front() ) );
}

*///

PyObject*
ConnectionService_purgeConnectionPool( PyObject* self )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;
  if ( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error : No Valid C++ object" );
    return 0;
  }
  try {
    py_this->object->purgeConnectionPool();
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when purging the connection pool" );
    return 0;
  }

  Py_RETURN_NONE;
}


PyObject*
ConnectionService_configuration( PyObject* self )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;
  if ( !py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error : No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IConnectionServiceConfiguration* theConfiguration = &( py_this->object->configuration() );
    coral::PyCoral::IConnectionServiceConfiguration* ob = PyObject_New( coral::PyCoral::IConnectionServiceConfiguration, coral::PyCoral::IConnectionServiceConfiguration_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a configuration object" );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theConfiguration, "name", 0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if ( ok )
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IConnectionServiceConfiguration Object" );
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
                     (char*) "Error when retrieving the configuration object" );
    return 0;
  }
}


PyObject*
ConnectionService_webCacheControl( PyObject* self )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;
  if ( !py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error : No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IWebCacheControl* theWebCacheControl = &( py_this->object->webCacheControl() );
    coral::PyCoral::IWebCacheControl* ob = PyObject_New( coral::PyCoral::IWebCacheControl,
                                                         coral::PyCoral::IWebCacheControl_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a webCacheControl object" );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theWebCacheControl, "name", 0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IWebCacheControl Object" );
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
                     (char*) "Error when retrieving the webCacheControl object" );
    return 0;
  }
}


PyObject*
ConnectionService_monitoringReporter( PyObject* self )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;
  if ( !py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error : No Valid C++ Object" );
    return 0;
  }
  try {
    coral::IMonitoringReporter* theMonitoringReporter = const_cast<coral::IMonitoringReporter*>(&( py_this->object->monitoringReporter()) );
    coral::PyCoral::IMonitoringReporter* ob = PyObject_New( coral::PyCoral::IMonitoringReporter,
                                                            coral::PyCoral::IMonitoringReporter_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Monitoring Reporter object" );
      return 0;
    }
    PyObject* c_object = PyCapsule_New( theMonitoringReporter, "name", 0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize IMonitoringReporter Object" );
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
                     (char*) "Error when retrieving the MonitoringReporter object" );
    return 0;
  }
}

PyObject*
ConnectionService_connect( PyObject* self, PyObject* args, PyObject* kwds )
{
  coral::PyCoral::ConnectionService* py_this = (coral::PyCoral::ConnectionService*) self;

  if ( !py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error : No Valid C++ Object" );
    return 0;
  }

  try {
    char* connectionName = 0;
    char* roleName = 0;
    coral::AccessMode mode = coral::Update;
    char* keywords[] = { (char*) "connectionName", (char*) "role", (char*) "accessMode", 0 };

    int numberOfArguments = PyTuple_GET_SIZE( args );
    int numberOfKeyWords = ( kwds ) ? PyTuple_GET_SIZE( kwds ) : 0;

    if ( numberOfArguments == 0 && numberOfKeyWords == 0 ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       "Connection string not specified" );
      return 0;
    }


    if ( numberOfArguments == 0 ) {
      if( ! PyArg_ParseTupleAndKeywords( args, kwds, (char*) "|ssi", keywords, &connectionName, &roleName, &mode ) ) {
        return 0;
      }
    }
    else if ( numberOfArguments == 1 ) {
      if( ! PyArg_ParseTupleAndKeywords( args, kwds, (char*) "s|si", keywords, &connectionName, &roleName, &mode ) ) {
        return 0;
      }
    }
    else if (  numberOfArguments == 2 ) {
      PyObject* secondItem = PyTuple_GET_ITEM( args, 1 );
      //Py_INCREF( secondItem );
      
      #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
      bool isInteger = ( PyInt_Check( secondItem ) ? true : false );
      #else
      bool isInteger = ( PyLong_Check( secondItem ) ? true : false );
      #endif

      //Py_DECREF( secondItem );
      if ( isInteger ) {
        char* localkeywords[] = { (char*) "connectionName", (char*) "accessMode", (char*) "role", 0 };
        if( ! PyArg_ParseTupleAndKeywords( args, kwds, (char*) "si|s", localkeywords,
                                           &connectionName, &mode, &roleName ) ) {
          return 0;
        }
      }
      else {
        if( ! PyArg_ParseTupleAndKeywords( args, kwds, (char*) "ss|i", keywords, &connectionName, &roleName, &mode ) ) {
          return 0;
        }
      }
    }
    else {
      if( ! PyArg_ParseTupleAndKeywords( args, kwds, (char*) "ssi", keywords, &connectionName, &roleName, &mode ) ) {
        return 0;
      }
    }

    if ( ! connectionName ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       "Connection string not specified" );
      return 0;
    }

    coral::ISessionProxy* theSessionProxy=0;
    if ( roleName )
      theSessionProxy = py_this->object->connect( std::string( connectionName ), std::string( roleName ), mode );
    else
      theSessionProxy = py_this->object->connect( std::string( connectionName ), mode );

    coral::PyCoral::ISessionProxy* ob = PyObject_New( coral::PyCoral::ISessionProxy,
                                                      coral::PyCoral::ISessionProxy_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating a Session Proxy object" );
      return 0;
    }

    PyObject* c_object = PyCapsule_New( theSessionProxy, "name", 0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( Py_TYPE(ob)->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ISessionProxy Object" );
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
                     (char*) "Error when creating Session Proxy object" );
    return 0;
  }
}
