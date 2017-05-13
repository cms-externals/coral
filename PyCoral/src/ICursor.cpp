#include "Exception.h"
#include "ICursor.h"
#include "ICursorIterator.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/ICursor.h"
#include <sstream>

// Forward declaration of the methods
static int ICursor_init( PyObject* self, PyObject* args, PyObject* kwds );
static void ICursor_dealloc( PyObject* self );
static PyObject* ICursor_iter( PyObject* self );
//static PyObject* ICursor_str( PyObject* self );

static PyObject* ICursor_next( PyObject* self );
static PyObject* ICursor_currentRow( PyObject* self );
static PyObject* ICursor_close( PyObject* self );


// Type definition
PyTypeObject*
coral::PyCoral::ICursor_Type()
{
  static PyMethodDef ICursor_Methods[] = {
    { (char*) "next", (PyCFunction) ICursor_next, METH_NOARGS,
      (char*) "Positions the cursor to the next available row in the result set. If there are no more rows in the result set false is returned." },
    { (char*) "currentRow", (PyCFunction) ICursor_currentRow, METH_NOARGS,
      (char*) "Returns a reference to output buffer holding the data of the last row fetched." },
    { (char*) "close", (PyCFunction) ICursor_close, METH_NOARGS,
      (char*) "Explicitly closes the cursor, releasing the resources on the server." },
    {0, 0, 0, 0}
  };

  static char ICursor_doc[] = "Interface for the iteration over the result set of a query.";

  static PyTypeObject ICursor_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.ICursor", /*tp_name*/
    sizeof(coral::PyCoral::ICursor), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    ICursor_dealloc, /*tp_dealloc*/
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
    ICursor_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    ICursor_iter, /*tp_iter*/
    0, /*tp_iternext*/
    ICursor_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    ICursor_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    ICursor_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &ICursor_Type;
}

int
ICursor_init( PyObject* self, PyObject*  args, PyObject* /*kwds*/ )
{
  coral::PyCoral::ICursor* py_this = (coral::PyCoral::ICursor*) self;
  if( ! py_this ) {
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
  py_this->object = static_cast<coral::ICursor*>
    ( PyCObject_AsVoidPtr( c_object ) );
  if ( py_this->parent ) Py_INCREF( py_this->parent );
  return 0;
}

void
ICursor_dealloc( PyObject* self )
//destructor of the python object
{
  //get the PyCoral object from the self pointer
  coral::PyCoral::ICursor* py_this = (coral::PyCoral::ICursor*) self;
  //check if we have an object
  if(py_this->object)
    //yes we have
  {
    py_this->object = 0;
  }

  if ( py_this->parent ) {
    Py_DECREF( py_this->parent );
  }
  py_this->parent = 0;
  self->ob_type->tp_free( self );
}

PyObject*
ICursor_next( PyObject* self )
{
  coral::PyCoral::ICursor* py_this = (coral::PyCoral::ICursor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    if ( py_this->object->next() ) Py_RETURN_TRUE;
    else Py_RETURN_FALSE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when retrieving next cursor position" );
    return 0;
  }
}


PyObject*
ICursor_currentRow( PyObject* self )
{
  coral::PyCoral::ICursor* py_this = (coral::PyCoral::ICursor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    coral::AttributeList* theAttrList = const_cast<coral::AttributeList*>(&( py_this->object->currentRow() )) ;
    coral::PyCoral::AttributeList* ob = PyObject_New( coral::PyCoral::AttributeList, coral::PyCoral::AttributeList_Type() );
    if ( ! ob ) {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error when creating AttributeList object." );
      return 0;
    }
    PyObject* c_object = PyCObject_FromVoidPtr( theAttrList,0 );
    PyObject* temp = Py_BuildValue((char*)"OO", py_this, c_object );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);
    Py_DECREF( c_object );
    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize AttributeList Object" );
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
                     (char*) "Error when retrieving the current Row" );
    return 0;
  }
}

PyObject*
ICursor_close( PyObject* self )
{
  coral::PyCoral::ICursor* py_this = (coral::PyCoral::ICursor*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    py_this->object->close();
    Py_RETURN_NONE;
  }
  catch( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error when closing the cursor" );
    return 0;
  }
}

PyObject*
ICursor_iter( PyObject* self )
{
  try {
    coral::PyCoral::ICursorIterator* ob = PyObject_New( coral::PyCoral::ICursorIterator,
                                                        coral::PyCoral::ICursorIterator_Type() );
    if ( ! ob )
    {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*) "Error in Creating ICursorIterator object." );
      return 0;
    }
    PyObject* temp = Py_BuildValue((char*)"O", self );
    bool ok = ( ob->ob_type->tp_init( (PyObject*) ob,temp,0)==0);
    Py_DECREF(temp);

    if (ok)
      return (PyObject*) ob;
    else {
      PyErr_SetString( coral::PyCoral::Exception(),
                       (char*)"Could not Initialize ICursorIterator Object" );
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
                     (char*)"Error: In ICursorIterator" );
    return 0;
  }
}
