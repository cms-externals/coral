#include "Exception.h"
#include "PyCoral/TimeStamp.h"
#include "CoralBase/TimeStamp.h"
#include <sstream>

// Forward declaration of the methods
static int TimeStamp_init( PyObject* self, PyObject* args, PyObject* kwds );
static void TimeStamp_dealloc( PyObject* self );
static int TimeStamp_compare( PyObject* obj1, PyObject* obj2 );
static PyObject* TimeStamp_str( PyObject* self );

static PyObject* TimeStamp_year( PyObject* self );
static PyObject* TimeStamp_month( PyObject* self );
static PyObject* TimeStamp_day( PyObject* self );
static PyObject* TimeStamp_hour( PyObject* self );
static PyObject* TimeStamp_minute( PyObject* self );
static PyObject* TimeStamp_second( PyObject* self );
static PyObject* TimeStamp_nanosecond( PyObject* self );

// Type definition
PyTypeObject*
coral::PyCoral::TimeStamp_Type()
{
  static PyMethodDef TimeStamp_Methods[] = {
    { (char*) "year", (PyCFunction) TimeStamp_year, METH_NOARGS,
      (char*) "Returns the year of the timestamp" },
    { (char*) "month", (PyCFunction) TimeStamp_month, METH_NOARGS,
      (char*) "Returns the month of the timestamp" },
    { (char*) "day", (PyCFunction) TimeStamp_day, METH_NOARGS,
      (char*) "Returns the day of the timestamp" },
    { (char*) "hour", (PyCFunction) TimeStamp_hour, METH_NOARGS,
      (char*) "Returns the hour of the timestamp" },
    { (char*) "minute", (PyCFunction) TimeStamp_minute, METH_NOARGS,
      (char*) "Returns minute of the timestamp" },
    { (char*) "second", (PyCFunction) TimeStamp_second, METH_NOARGS,
      (char*) "Returns the second of the timestamp" },
    { (char*) "nanosecond", (PyCFunction) TimeStamp_nanosecond, METH_NOARGS,
      (char*) "Returns the nanosecond" },
    {0, 0, 0, 0}
  };

  static char TimeStamp_doc[] = "A class defining the ANSI DATE TIME type. The default constructor constructs the current date & time.";

  static PyTypeObject TimeStamp_Type = {
    PyObject_HEAD_INIT(0)
    0, /*ob_size*/
    (char*) "coral.TimeStamp", /*tp_name*/
    sizeof(coral::PyCoral::TimeStamp), /*tp_basicsize*/
    0, /*tp_itemsize*/
       /* methods */
    TimeStamp_dealloc, /*tp_dealloc*/
    0, /*tp_print*/
    0, /*tp_getattr*/
    0, /*tp_setattr*/
    TimeStamp_compare, /*tp_compare*/
    0, /*tp_repr*/
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash*/
    0, /*tp_call*/
    TimeStamp_str, /*tp_str*/
    PyObject_GenericGetAttr, /*tp_getattro*/
    PyObject_GenericSetAttr, /*tp_setattro*/
    0, /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT, /*tp_flags*/
    TimeStamp_doc, /*tp_doc*/
    0, /*tp_traverse*/
    0, /*tp_clear*/
    0, /*tp_richcompare*/
    0, /*tp_weaklistoffset*/
    0, /*tp_iter*/
    0, /*tp_iternext*/
    TimeStamp_Methods, /*tp_methods*/
    0, /*tp_members*/
    0, /*tp_getset*/
    0, /*tp_base*/
    0, /*tp_dict*/
    0, /*tp_descr_get*/
    0, /*tp_descr_set*/
    0, /*tp_dictoffset*/
    TimeStamp_init, /*tp_init*/
    PyType_GenericAlloc, /*tp_alloc*/
    PyType_GenericNew, /*tp_new*/
    _PyObject_Del, /*tp_free*/
    0, /*tp_is_gc*/
    0, /*tp_bases*/
    0, /*tp_mro*/
    0, /*tp_cache*/
    0, /*tp_subclasses*/
    0, /*tp_weaklist*/
    TimeStamp_dealloc /*tp_del*/
#if PY_VERSION_HEX >= 0x02060000
    ,0 /*tp_version_tag*/
#endif
  };
  return &TimeStamp_Type;
}

int
TimeStamp_init( PyObject* self, PyObject* args, PyObject* kwds )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if ( !py_this ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Self is NULL" );

    return -1;
  }
  py_this->object = 0;
  py_this->parent = 0;

  int numberOfArguments = PyTuple_GET_SIZE( args );
  int numberOfKeyWords = ( kwds ) ? PyTuple_GET_SIZE( kwds ) : 0;

  try{
    if ( numberOfArguments == 0 && numberOfKeyWords == 0 ) { // If no arguments are passed it creates the current date and time
      py_this->object = new coral::TimeStamp( coral::TimeStamp::now() );
    }
    else {
      if ( (numberOfKeyWords == 0)  && (numberOfArguments == 2) ) {
        if ( ! PyNumber_Check( PyTuple_GET_ITEM(args,0) )) {
          PyObject* c_object = 0;
          if ( !PyArg_ParseTuple( args, (char*)"OO",
                                  &(py_this->parent),
                                  &c_object ) ) return -1;
          py_this->object = static_cast<coral::TimeStamp*>
            ( PyCObject_AsVoidPtr( c_object ) );
          if ( py_this->parent ) Py_INCREF( py_this->parent );
          return 0;
        }
      }
      char* format = (char *) "i|iiiiil"; // The year at least should be specified
      char* keywords[] = { (char*) "year", (char*) "month", (char*) "day", (char*) "hour", (char*) "minute", (char*) "second", (char*) "nanosecond", 0 };
      int year = 0;
      int month = 1;
      int day = 1;
      int hour = 0;
      int minute = 0;
      int second = 0;
      long nanosecond = 0;
      if( PyArg_ParseTupleAndKeywords( args, kwds, format, keywords, &year, &month, &day, &hour, &minute, &second, &nanosecond ) ) {
        py_this->object = new coral::TimeStamp( year, month, day, hour, minute, second, nanosecond );
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
                     "Could not create a new TimeStamp" );
    return -1;
  }
  return 0;
}



void
TimeStamp_dealloc( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
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
TimeStamp_compare( PyObject* obj1, PyObject* obj2 )
{
  coral::PyCoral::TimeStamp* d1 = (coral::PyCoral::TimeStamp*) obj1;
  coral::PyCoral::TimeStamp* d2 = (coral::PyCoral::TimeStamp*) obj2;
  if ( !d1->object || !d2->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: Bad C++ Object" );
    return -1;
  }
  try{
    coral::TimeStamp& time1 = *( d1->object );
    coral::TimeStamp& time2 = *( d2->object );
    if ( time1.year() < time2.year() ) return -1;
    if ( time1.year() > time2.year() ) return 1;
    if ( time1.month() < time2.month() ) return -1;
    if ( time1.month() > time2.month() ) return 1;
    if ( time1.day() < time2.day() ) return -1;
    if ( time1.day() > time2.day() ) return 1;
    if ( time1.hour() < time2.hour() ) return -1;
    if ( time1.hour() > time2.hour() ) return 1;
    if ( time1.minute() < time2.minute() ) return -1;
    if ( time1.minute() > time2.minute() ) return 1;
    if ( time1.second() < time2.second() ) return -1;
    if ( time1.second() > time2.second() ) return 1;
    if ( time1.nanosecond() < time2.nanosecond() ) return -1;
    if ( time1.nanosecond() > time2.nanosecond() ) return 1;
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return -1;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not compare the TimeStamps" );
    return -1;
  }
  return 0;
}


PyObject*
TimeStamp_str( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  std::ostringstream os;
  coral::TimeStamp& time = *( py_this->object );
  os << time.year() << "/";
  int month = time.month();
  if ( month < 10 ) os << "0";
  os << month << "/";
  int day = time.day();
  if ( day < 10 ) os << "0";
  os << day << ":";
  int hour = time.hour();
  if ( hour < 10 ) os << "0";
  os << hour << ":";
  int minute = time.minute();
  if ( minute < 10 ) os << "0";
  os << minute << ":";
  int second = time.second();
  if ( second < 10 ) os << "0";
  os << second << ":";
  long nanosecond = time.nanosecond();
  os << nanosecond;
  std::string result = os.str();
  return Py_BuildValue( (char*) "s", result.c_str() );
}


PyObject*
TimeStamp_year( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
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
TimeStamp_month( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
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
TimeStamp_day( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
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


PyObject*
TimeStamp_hour( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int hour = py_this->object->hour();
    return Py_BuildValue( (char*) "i", hour );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive hour" );
    return 0;
  }
}


PyObject*
TimeStamp_minute( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int minute = py_this->object->minute();
    return Py_BuildValue( (char*) "i", minute );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive minute" );
    return 0;
  }
}

PyObject*
TimeStamp_second( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    int second = py_this->object->second();
    return Py_BuildValue( (char*) "i", second );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive second" );
    return 0;
  }
}


PyObject*
TimeStamp_nanosecond( PyObject* self )
{
  coral::PyCoral::TimeStamp* py_this = (coral::PyCoral::TimeStamp*) self;
  if( ! py_this->object ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*) "Error: No Valid C++ Object" );
    return 0;
  }
  try {
    long nanosecond = py_this->object->nanosecond();
    return Py_BuildValue((char*)"k", nanosecond );
  }
  catch ( std::exception& e ) {
    PyErr_SetString( coral::PyCoral::Exception(), e.what() );
    return 0;
  }
  catch ( ... ) {
    PyErr_SetString( coral::PyCoral::Exception(),
                     (char*)"Could not retreive nanosecond" );
    return 0;
  }
}
