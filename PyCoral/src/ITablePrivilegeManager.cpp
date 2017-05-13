#include "Exception.h"
#include "ITablePrivilegeManager.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include <sstream>

// Forward declaration of the methods
static int ITablePrivilegeManager_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ITablePrivilegeManager_dealloc( PyObject* self );
//static PyObject* ITablePrivilegeManager_str( PyObject* self );

static PyObject* ITablePrivilegeManager_grantToUser( PyObject* self, PyObject* args );
static PyObject* ITablePrivilegeManager_revokeFromUser( PyObject* self, PyObject* args );
static PyObject* ITablePrivilegeManager_grantToPublic( PyObject* self, PyObject* args );
static PyObject* ITablePrivilegeManager_revokeFromPublic( PyObject* self, PyObject* args );


// Type definition
PyTypeObject*
coral::PyCoral::ITablePrivilegeManager_Type()
{
  static PyMethodDef ITablePrivilegeManager_Methods[] = {
    { (char*) "grantToUser", (PyCFunction) ITablePrivilegeManager_grantToUser, METH_VARARGS,
      (char*) "Grants an access right to a specific user." },
    { (char*) "revokeFromUser", (PyCFunction) ITablePrivilegeManager_revokeFromUser, METH_VARARGS,
      (char*) "Revokes a right from the specified user." },
    { (char*) "grantToPublic", (PyCFunction) ITablePrivilegeManager_grantToPublic, METH_VARARGS,
      (char*) "Grants the specified right to all users." },
    { (char*) "revokeFromPublic", (PyCFunction) ITablePrivilegeManager_revokeFromPublic, METH_VARARGS,
      (char*) "Revokes the specified right from all users." },
    {0, 0, 0, 0}
  };

  static char ITablePrivilegeManager_doc[] = "Interface for managing the privileges in a table.";

  static PyTypeObject ITablePrivilegeManager_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ITablePrivilegeManager", /*tp_name*/
    sizeof(coral::PyCoral::ITablePrivilegeManager), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ITablePrivilegeManager_dealloc, /*tp_dealloc*/
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
    0, /*tp_str*/
    PyObject_GenericGetAttr, /*tp_getattro*/
    PyObject_GenericSetAttr, /*tp_setattro*/
    0, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    ITablePrivilegeManager_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    ITablePrivilegeManager_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ITablePrivilegeManager_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ITablePrivilegeManager_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ITablePrivilegeManager_Type;
}

int
ITablePrivilegeManager_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
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
  py_this->object = static_cast<coral::ITablePrivilegeManager*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if (py_this->parent) Py_INCREF (py_this->parent);
  return 0;
}


void
ITablePrivilegeManager_dealloc( PyObject* self )
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
  py_this->object = 0;
  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}


PyObject*
ITablePrivilegeManager_grantToUser( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* userName;
    coral::ITablePrivilegeManager::Privilege privilege = coral::ITablePrivilegeManager::Select;
    if ( ! PyArg_ParseTuple(args, (char*) "si", &userName, &privilege) ) return 0;
    py_this->object->grantToUser(std::string(userName), privilege);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when granting privilege to user" );
    return 0;
  }
}


PyObject*
ITablePrivilegeManager_revokeFromUser( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    char* userName;
    coral::ITablePrivilegeManager::Privilege privilege = coral::ITablePrivilegeManager::Select;
    if ( ! PyArg_ParseTuple(args, (char*) "si", &userName, &privilege) ) return 0;
    py_this->object->revokeFromUser(std::string(userName), privilege);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when revoking privilege from user" );
    return 0;
  }
}


PyObject*
ITablePrivilegeManager_grantToPublic( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITablePrivilegeManager::Privilege privilege = coral::ITablePrivilegeManager::Select;
    if ( ! PyArg_ParseTuple(args, (char*)"i", &privilege)) return 0;
    py_this->object->grantToPublic(privilege);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when granting privilege to public" );
    return 0;
  }
}


PyObject*
ITablePrivilegeManager_revokeFromPublic( PyObject* self, PyObject* args)
{
  coral::PyCoral::ITablePrivilegeManager* py_this = (coral::PyCoral::ITablePrivilegeManager*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::ITablePrivilegeManager::Privilege privilege = coral::ITablePrivilegeManager::Select;
    if ( ! PyArg_ParseTuple(args, (char*)"i", &privilege)) return 0;
    py_this->object->revokeFromPublic(privilege);
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when revoking privilege to public" );
    return 0;
  }
}
