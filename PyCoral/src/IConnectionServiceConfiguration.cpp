#include "IConnectionServiceConfiguration.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "Exception.h"
#include <exception>

#if PY_MAJOR_VERSION >= 3
    #define PyString_Check PyUnicode_Check
    #define PyString_AsString PyUnicode_AsUTF8
#endif
// Ignore 'dereferencing type-punned pointer' warnings caused by
// Py_RETURN_TRUE/FALSE (CMS patch for sr #141482 and bug #89768)
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

// Forward declaration of the methods
static int IConnectionServiceConfiguration_init( PyObject* self, PyObject* args, PyObject* kwds );
static void IConnectionServiceConfiguration_dealloc( PyObject* self );
static PyObject* IConnectionServiceConfiguration_enableReplicaFailOver( PyObject* self );
static PyObject* IConnectionServiceConfiguration_disableReplicaFailOver( PyObject* self );
static PyObject* IConnectionServiceConfiguration_isReplicaFailOverEnabled( PyObject* self );
static PyObject* IConnectionServiceConfiguration_enableConnectionSharing( PyObject* self );
static PyObject* IConnectionServiceConfiguration_disableConnectionSharing( PyObject* self );
static PyObject* IConnectionServiceConfiguration_isConnectionSharingEnabled( PyObject* self );
static PyObject* IConnectionServiceConfiguration_enableReadOnlySessionOnUpdateConnections( PyObject* self );
static PyObject* IConnectionServiceConfiguration_disableReadOnlySessionOnUpdateConnections( PyObject* self );
static PyObject* IConnectionServiceConfiguration_isReadOnlySessionOnUpdateConnectionsEnabled( PyObject* self );
static PyObject* IConnectionServiceConfiguration_setConnectionRetrialPeriod( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_connectionRetrialPeriod( PyObject* self );
static PyObject* IConnectionServiceConfiguration_setConnectionRetrialTimeOut( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_connectionRetrialTimeOut( PyObject* self );
static PyObject* IConnectionServiceConfiguration_setConnectionTimeOut( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_connectionTimeOut( PyObject* self );
static PyObject* IConnectionServiceConfiguration_setMissingConnectionExclusionTime( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_missingConnectionExclusionTime( PyObject* self );
static PyObject* IConnectionServiceConfiguration_enablePoolAutomaticCleanUp( PyObject* self );
static PyObject* IConnectionServiceConfiguration_disablePoolAutomaticCleanUp( PyObject* self );
static PyObject* IConnectionServiceConfiguration_isPoolAutomaticCleanUpEnabled( PyObject* self );
static PyObject* IConnectionServiceConfiguration_setDefaultAuthenticationService( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_setDefaultLookupService( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_setDefaultRelationalService( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_setDefaultMonitoringService( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_setMonitoringLevel( PyObject* self, PyObject* args );
static PyObject* IConnectionServiceConfiguration_monitoringLevel( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::IConnectionServiceConfiguration_Type()
{
  static PyMethodDef IConnectionServiceConfiguration_Methods[] = {
    { (char*) "enableReplicaFailOver", (PyCFunction)(void *) IConnectionServiceConfiguration_enableReplicaFailOver, METH_NOARGS,
      (char*) "Enables the failing over to the next available replica in case the current one is not available, otherwise the ConnectionService gives up." },
    { (char*) "disableReplicaFailOver", (PyCFunction)(void *) IConnectionServiceConfiguration_disableReplicaFailOver, METH_NOARGS,
      (char*) "Disables the failing over to the next available replica in case the current one is not available." },
    { (char*) "isReplicaFailOverEnabled", (PyCFunction)(void *) IConnectionServiceConfiguration_isReplicaFailOverEnabled, METH_NOARGS,
      (char*) "Returns the failover mode." },
    { (char*) "enableConnectionSharing", (PyCFunction)(void *) IConnectionServiceConfiguration_enableConnectionSharing, METH_NOARGS,
      (char*) "Enables the sharing of the same physical connection among more clients." },
    { (char*) "disableConnectionSharing", (PyCFunction)(void *) IConnectionServiceConfiguration_disableConnectionSharing, METH_NOARGS,
      (char*) "Disnables the sharing of the same physical connection among more clients." },
    { (char*) "isConnectionSharingEnabled", (PyCFunction)(void *) IConnectionServiceConfiguration_isConnectionSharingEnabled, METH_NOARGS,
      (char*) "Returns the connection sharing mode." },
    { (char*) "enableReadOnlySessionOnUpdateConnections", (PyCFunction)(void *) IConnectionServiceConfiguration_enableReadOnlySessionOnUpdateConnections, METH_NOARGS,
      (char*) "Enables the re-use of Update connections for Read-Only sessions." },
    { (char*) "disableReadOnlySessionOnUpdateConnections", (PyCFunction)(void *) IConnectionServiceConfiguration_disableReadOnlySessionOnUpdateConnections, METH_NOARGS,
      (char*) "Disables the re-use of Update connections for Read-Only sessions." },
    { (char*) "isReadOnlySessionOnUpdateConnectionsEnabled", (PyCFunction)(void *) IConnectionServiceConfiguration_isReadOnlySessionOnUpdateConnectionsEnabled, METH_NOARGS,
      (char*) "Returns true if the  re-use of Update connections for Read-Only sessions is enabled; false otherwise." },
    { (char*) "setConnectionRetrialPeriod", (PyCFunction)(void *) IConnectionServiceConfiguration_setConnectionRetrialPeriod, METH_O,
      (char*) "Sets the period of connection retrials (time interval between two retrials)." },
    { (char*) "connectionRetrialPeriod", (PyCFunction)(void *) IConnectionServiceConfiguration_connectionRetrialPeriod, METH_NOARGS,
      (char*) "Returns the rate of connection retrials (time interval between two retrials)." },
    { (char*) "setConnectionRetrialTimeOut", (PyCFunction)(void *) IConnectionServiceConfiguration_setConnectionRetrialTimeOut, METH_O,
      (char*) "Sets the time out for the connection retrials before the connection service fails over to the next available replica or quits." },
    { (char*) "connectionRetrialTimeOut", (PyCFunction)(void *) IConnectionServiceConfiguration_connectionRetrialTimeOut, METH_NOARGS,
      (char*) "Returns the time out for the connection retrials before the connection service fails over to the next available replica or quits." },
    { (char*) "setConnectionTimeOut", (PyCFunction)(void *) IConnectionServiceConfiguration_setConnectionTimeOut, METH_O,
      (char*) "Sets the connection time out in seconds." },
    { (char*) "connectionTimeOut", (PyCFunction)(void *) IConnectionServiceConfiguration_connectionTimeOut, METH_NOARGS,
      (char*) "Retrieves the connection time out in seconds." },
    { (char*) "setMissingConnectionExclusionTime", (PyCFunction)(void *) IConnectionServiceConfiguration_setMissingConnectionExclusionTime, METH_O,
      (char*) "Sets the time duration of exclusion from failover list for a connection not available." },
    { (char*) "missingConnectionExclusionTime", (PyCFunction)(void *) IConnectionServiceConfiguration_missingConnectionExclusionTime, METH_NOARGS,
      (char*) "Retrieves the time duration of exclusion from failover list for a connection not available." },
    { (char*) "enablePoolAutomaticCleanUp", (PyCFunction)(void *) IConnectionServiceConfiguration_enablePoolAutomaticCleanUp, METH_NOARGS,
      (char*) "Activate the parallel thread for idle pool cleaning up." },
    { (char*) "disablePoolAutomaticCleanUp", (PyCFunction)(void *) IConnectionServiceConfiguration_disablePoolAutomaticCleanUp, METH_NOARGS,
      (char*) "Disable the parallel thread for idle pool cleaning up." },
    { (char*) "isPoolAutomaticCleanUpEnabled", (PyCFunction)(void *) IConnectionServiceConfiguration_isPoolAutomaticCleanUpEnabled, METH_NOARGS,
      (char*) "Returns true if the parallel thread for idle pool cleaning up is enabled." },
    { (char*) "setDefaultAuthenticationService", (PyCFunction)(void *) IConnectionServiceConfiguration_setDefaultAuthenticationService, METH_O,
      (char*) "Sets the default authentication service implementation." },
    { (char*) "setDefaultLookupService", (PyCFunction)(void *) IConnectionServiceConfiguration_setDefaultLookupService, METH_O,
      (char*) "Sets the default lookup service implementation." },
    { (char*) "setDefaultRelationalService", (PyCFunction)(void *) IConnectionServiceConfiguration_setDefaultRelationalService, METH_O,
      (char*) "Sets the default relational service implementation." },
    { (char*) "setDefaultMonitoringService", (PyCFunction)(void *) IConnectionServiceConfiguration_setDefaultMonitoringService, METH_O,
      (char*) "Sets the default monitoring service implementation." },
    { (char*) "setMonitoringLevel", (PyCFunction)(void *) IConnectionServiceConfiguration_setMonitoringLevel, METH_O,
      (char*) "Sets the monitoring level for subsequent connections." },
    { (char*) "monitoringLevel", (PyCFunction)(void *) IConnectionServiceConfiguration_monitoringLevel, METH_NOARGS,
      (char*) "Retrieves the monitoring level." },
    {0, 0, 0, 0}
  };

  static char IConnectionServiceConfiguration_doc[] = "Interface for configuring the connection service.";

  static PyTypeObject IConnectionServiceConfiguration_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.IConnectionServiceConfiguration", // tp_name
    sizeof(coral::PyCoral::IConnectionServiceConfiguration), // tp_basicsize
    0, // tp_itemsize
       //  methods
    IConnectionServiceConfiguration_dealloc, // tp_dealloc
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
    IConnectionServiceConfiguration_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear
    0, // tp_richcompare
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    IConnectionServiceConfiguration_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    IConnectionServiceConfiguration_init, // tp_init
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
    IConnectionServiceConfiguration_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &IConnectionServiceConfiguration_Type;
}


int
IConnectionServiceConfiguration_init( PyObject* self, PyObject* args , PyObject* /* kwds */ )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
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
  py_this->object = static_cast<coral::IConnectionServiceConfiguration*>( PyCapsule_GetPointer( c_object , "name") );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}


void
IConnectionServiceConfiguration_dealloc( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }

  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
IConnectionServiceConfiguration_enableReplicaFailOver( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->enableReplicaFailOver();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when enabling replica fail-over" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_disableReplicaFailOver( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->disableReplicaFailOver();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when disabling replica fail-over" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_isReplicaFailOverEnabled( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isReplicaFailoverEnabled() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving replica fail-over mode" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_enableConnectionSharing( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->enableConnectionSharing();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when enabling connection sharing" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_disableConnectionSharing( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->disableConnectionSharing();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when disabling connection sharing" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_isConnectionSharingEnabled( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isConnectionSharingEnabled() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the connection sharing mode." );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_enableReadOnlySessionOnUpdateConnections( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->enableReadOnlySessionOnUpdateConnections();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when enabling read-only sessions on update connections" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_disableReadOnlySessionOnUpdateConnections( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->disableReadOnlySessionOnUpdateConnections();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when disabling read-only sessions on update connections" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_isReadOnlySessionOnUpdateConnectionsEnabled( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isReadOnlySessionOnUpdateConnectionsEnabled() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the readonlyOnUpdateConnections mode." );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_setConnectionRetrialPeriod( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setConnectionRetrialPeriod( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setConnectionRetrialPeriod( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Connection Retrial Period." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_connectionRetrialPeriod( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = py_this->object->connectionRetrialPeriod();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the connectionRetrialPeriod." );
    return 0;
  }
}



PyObject*
IConnectionServiceConfiguration_setConnectionRetrialTimeOut( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setConnectionRetrialTimeOut( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setConnectionRetrialTimeOut( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Connection Retrial TimeOut." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_connectionRetrialTimeOut( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = py_this->object->connectionRetrialTimeOut();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the connectionRetrialTimeOut." );
    return 0;
  }
}



PyObject*
IConnectionServiceConfiguration_setConnectionTimeOut( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setConnectionTimeOut( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setConnectionTimeOut( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Connection TimeOut." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_connectionTimeOut( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = py_this->object->connectionTimeOut();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the connectionTimeOut." );
    return 0;
  }
}



PyObject*
IConnectionServiceConfiguration_setMissingConnectionExclusionTime( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setMissingConnectionExclusionTime( PyLong_AsLong(args) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setMissingConnectionExclusionTime( PyInt_AS_LONG(args) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an Long integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting Missing Connection Exclusion Time." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_missingConnectionExclusionTime( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = py_this->object->missingConnectionExclusionTime();
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the missing connection exclusion Time." );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_enablePoolAutomaticCleanUp( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->enablePoolAutomaticCleanUp();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when enabling Pool Automatic CleanUp" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_disablePoolAutomaticCleanUp( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->disablePoolAutomaticCleanUp();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when disabling Pool Automatic CleanUp" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_isPoolAutomaticCleanUpEnabled( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->isPoolAutomaticCleanUpEnabled() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving pool automatic cleanup mode" );
    return 0;
  }
}


PyObject*
IConnectionServiceConfiguration_setDefaultAuthenticationService( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    char* serviceName;
    if ( PyString_Check(args) ) serviceName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setAuthenticationService(std::string(serviceName) );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting the default authentication service." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_setDefaultLookupService( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    char* serviceName;
    if ( PyString_Check(args) ) serviceName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setLookupService(std::string(serviceName) );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting the default lookup service." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_setDefaultRelationalService( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    char* serviceName;
    if ( PyString_Check(args) ) serviceName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setRelationalService(std::string(serviceName) );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting the default relational service." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_setDefaultMonitoringService( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }

  try {
    char* serviceName;
    if ( PyString_Check(args) ) serviceName = PyString_AsString(args);
    else{
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error: Bad Argument Type" );
      return 0;
    }
    py_this->object->setMonitoringService(std::string(serviceName) );
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting the default monitoring service." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_setMonitoringLevel( PyObject* self, PyObject* args )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if (PyLong_Check( args ) ) {
      py_this->object->setMonitoringLevel( static_cast<coral::monitor::Level>( PyLong_AsLong(args) ) );
    }
    #if PY_VERSION_HEX <= 0x03000000  //CORALCOOL-2977
    else if ( PyInt_Check( args ) ) {
      py_this->object->setMonitoringLevel( static_cast<coral::monitor::Level>( PyInt_AS_LONG(args) ) );
    }
    #endif
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Argument is not an integer!" );
      return 0;
    }
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when setting the monitoring level." );
    return 0;
  }
  Py_RETURN_NONE;
}



PyObject*
IConnectionServiceConfiguration_monitoringLevel( PyObject* self )
{
  coral::PyCoral::IConnectionServiceConfiguration* py_this = (coral::PyCoral::IConnectionServiceConfiguration*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int result = static_cast<int>( py_this->object->monitoringLevel() );
    return Py_BuildValue((char*)"i",result);
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving the monitoring level." );
    return 0;
  }
}
