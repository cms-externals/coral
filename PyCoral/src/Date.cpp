#include "Exception.h"
#include "PyCoral/Date.h"
#include "CoralBase/Date.h"
#include <sstream>

// Forward declaration of the methods
static int Date_init( PyObject* self, PyObject* args, PyObject* kwds );
static void Date_dealloc( PyObject* self );
static int Date_compare( PyObject* obj1, PyObject* obj2 );
#if PY_MAJOR_VERSION >= 3
static PyObject* Date_rich_compare( PyObject *o1, PyObject* o2, int op );
#endif
static PyObject* Date_str( PyObject* self );

static PyObject* Date_year( PyObject* self );
static PyObject* Date_month( PyObject* self );
static PyObject* Date_day( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::Date_Type()
{
  static PyMethodDef Date_Methods[] = {
    { (char*) "year", (PyCFunction)(void *) Date_year, METH_NOARGS,
      (char*) "Returns the year of the date" },
    { (char*) "month", (PyCFunction)(void *) Date_month, METH_NOARGS,
      (char*) "Returns the month of the date [1-12]" },
    { (char*) "day", (PyCFunction)(void *) Date_day, METH_NOARGS,
      (char*) "Returns the day of the date [1-31]" },
    {0, 0, 0, 0}
  };

  static char Date_doc[] = "A class defining the ANSI DATE type. The default constructor constructs the current day.";

  static PyTypeObject Date_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    (char*) "coral.Date", // tp_name
    sizeof(coral::PyCoral::Date), // tp_basicsize
    0, // tp_itemsize
       //  methods
    Date_dealloc, // tp_dealloc
    0, // tp_print
    0, // tp_getattr
    0, // tp_setattr
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977
       Date_compare, // tp_compare
    #else
    0, // tp_compare
    #endif
    0, // tp_repr
    0, // tp_as_number
    0, // tp_as_sequence
    0, // tp_as_mapping
    0, // tp_hash
    0, // tp_call
    Date_str, // tp_str
    PyObject_GenericGetAttr, // tp_getattro
    PyObject_GenericSetAttr, // tp_setattro
    0, // tp_as_buffer
    Py_TPFLAGS_DEFAULT, // tp_flags
    Date_doc, // tp_doc
    0, // tp_traverse
    0, // tp_clear    
    #if PY_VERSION_HEX <= 0x03000000 //CORALCOOL-2977       
    0, // tp_richcompare
    #else
    (richcmpfunc)Date_rich_compare,
    #endif
    0, // tp_weaklistoffset
    0, // tp_iter
    0, // tp_iternext
    Date_Methods, // tp_methods
    0, // tp_members
    0, // tp_getset
    0, // tp_base
    0, // tp_dict
    0, // tp_descr_get
    0, // tp_descr_set
    0, // tp_dictoffset
    Date_init, // tp_init
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
    Date_dealloc // tp_del
    ,0 // tp_version_tag
    #if PY_MAJOR_VERSION >= 3
    ,0 //tp_finalize
    #endif
  };
  return &Date_Type;
}

int
Date_init( PyObject* self, PyObject* args, PyObject* kwds )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;

  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );
    return -1;
  }

  py_this->object = 0;
  py_this->parent = 0;

  int numberOfArguments = PyTuple_GET_SIZE( args );
  int numberOfKeyWords = ( kwds ) ? PyTuple_GET_SIZE( kwds ) : 0;

  try
  {
    // If no arguments are passed it creates the current date
    if ( numberOfArguments == 0 && numberOfKeyWords == 0 )
    {
      // Now UTC in CORAL3, was local time in previous implementation
      py_this->object = new coral::Date(); // UTC (task #51263)
      return 0;
    }
    else {
      if ( (numberOfKeyWords == 0)  && (numberOfArguments == 2) ) {
        if ( ! PyNumber_Check( PyTuple_GET_ITEM(args,0) )) {
          PyObject* c_object = 0;
          if ( !PyArg_ParseTuple( args, (char*)"OO",
                                  &(py_this->parent),
                                  &c_object ) ) return -1;
          py_this->object = static_cast<coral::Date*>
            ( PyCapsule_GetPointer( c_object , "name") );
          if ( py_this->parent ) Py_INCREF( py_this->parent );
          return 0;
        }
      }
      char* format = (char *) "i|ii"; // The year at least should be specified
      char* keywords[] = { (char*) "year", (char*) "month", (char*) "day", 0 };
      int year = 0;
      int month = 1;
      int day = 1;
      if( PyArg_ParseTupleAndKeywords( args, kwds, format, keywords, &year, &month, &day ) ) {
        py_this->object = new coral::Date( year, month, day );
        return 0;
      }
      else return -1;
    }
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not create a new Date" );
    return -1;
  }
}


void
Date_dealloc( PyObject* self )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;
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
Date_compare( PyObject* obj1, PyObject* obj2 )
{
  coral::PyCoral::Date* d1 = (coral::PyCoral::Date*) obj1;
  coral::PyCoral::Date* d2 = (coral::PyCoral::Date*) obj2;
  if ( !d1->object || !d2->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Bad C++ Object" );
    return -1;
  }
  try{
    coral::Date& date1 = *( d1->object );
    coral::Date& date2 = *( d2->object );
    if ( date1.year() < date2.year() ) return -1;
    if ( date1.year() > date2.year() ) return 1;
    if ( date1.month() < date2.month() ) return -1;
    if ( date1.month() > date2.month() ) return 1;
    if ( date1.day() < date2.day() ) return -1;
    if ( date1.day() > date2.day() ) return 1;
  }catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not compare the Dates" );
    return -1;
  }
  return 0;
}

#if PY_MAJOR_VERSION >= 3
  PyObject* Date_rich_compare(PyObject *o1, PyObject* o2, int op){
    if(o2==Py_None) Py_RETURN_FALSE;
    switch(op){
      case Py_EQ:{
         if(Date_compare(o1,o2) == 0) Py_RETURN_TRUE;
         Py_RETURN_FALSE;
      }
      default: Py_RETURN_NOTIMPLEMENTED;
    }
  }
#endif


PyObject*
Date_str( PyObject* self )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  std::ostringstream os;
  coral::Date& date = *( py_this->object );
  os << date.year() << "/";
  int month = date.month();
  if ( month < 10 ) os << "0";
  os << month << "/";
  int day = date.day();
  if ( day < 10 ) os << "0";
  os << day;
  std::string result = os.str();
  return Py_BuildValue( (char*) "s", result.c_str() );
}


PyObject*
Date_year( PyObject* self )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int year = py_this->object->year();
    return Py_BuildValue( (char*) "i", year );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive year" );
    return 0;
  }
}

PyObject*
Date_month( PyObject* self )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int month = py_this->object->month();
    return Py_BuildValue( (char*) "i", month );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive month" );
    return 0;
  }
}


PyObject*
Date_day( PyObject* self )
{
  coral::PyCoral::Date* py_this = (coral::PyCoral::Date*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int day = py_this->object->day();
    return Py_BuildValue( (char*) "i", day );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive day" );
    return 0;
  }
}
