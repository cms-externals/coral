#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif
#include "Exception.h"
#include "PyCoral/Date.h"
#include "PyCoral/TimeStamp.h"
#include "PyCoral/Blob.h"
#include "PyCoral/ConnectionService.h"
#include "IConnectionServiceConfiguration.h"
#include "ISessionProxy.h"
#include "ISchema.h"
#include "IQuery.h"
#include "IQueryDefinition.h"
#include "ITable.h"
#include "IPrimaryKey.h"
#include "IForeignKey.h"
#include "IUniqueConstraint.h"
#include "IIndex.h"
#include "IColumn.h"
#include "ICursor.h"
#include "IView.h"
#include "IViewFactory.h"
#include "ITransaction.h"
#include "ITypeConverter.h"
#include "ITableDataEditor.h"
#include "ITableSchemaEditor.h"
#include "ITableDescription.h"
#include "PyCoral/TableDescription.h"
#include "ITablePrivilegeManager.h"
#include "ISessionProperties.h"
#include "IOperationWithQuery.h"
#include "IBulkOperation.h"
#include "IBulkOperationWithQuery.h"
#include "IWebCacheControl.h"
#include "IWebCacheInfo.h"
#include "IMonitoringReporter.h"
#include "Attribute.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "RelationalAccess/IMonitoring.h"
#include "AttributeSpecification.h"
#include "AttributeListIterator.h"
#include "ICursorIterator.h"
#include "PyCoral/MessageStream.h"
#include "CoralBase/MessageStream.h"

// Module entry point
extern "C" {
#ifdef WIN32
  PyMODINIT_FUNC initlcg_PyCoral()
#else
    PyMODINIT_FUNC initliblcg_PyCoral()
#endif
  {
    // Check on the classes

    // Context class
    //    PyTypeObject* Context_Type = coral::PyCoral::Context_Type();
    //    if ( PyType_Ready( Context_Type ) < 0 ) return;

    // ConnectionService class
    PyTypeObject* ConnectionService_Type = coral::PyCoral::ConnectionService_Type();
    if ( PyType_Ready( ConnectionService_Type ) < 0 ) return;

    // IConnectionServiceConfiguration class
    if ( PyType_Ready( coral::PyCoral::IConnectionServiceConfiguration_Type() ) < 0 ) return;

    // Date class
    PyTypeObject* Date_Type = coral::PyCoral::Date_Type();
    if ( PyType_Ready( Date_Type ) < 0 ) return;

    // TimeStamp class
    PyTypeObject* TimeStamp_Type = coral::PyCoral::TimeStamp_Type();
    if ( PyType_Ready( TimeStamp_Type ) < 0 ) return;

    // Blob class
    PyTypeObject* Blob_Type = coral::PyCoral::Blob_Type();
    if ( PyType_Ready( Blob_Type ) < 0 ) return;

    // AttributeSpecification class
    if ( PyType_Ready( coral::PyCoral::AttributeSpecification_Type() ) < 0 ) return;

    // AttributeList class
    PyTypeObject* AttributeList_Type = coral::PyCoral::AttributeList_Type();
    if ( PyType_Ready( AttributeList_Type ) < 0 ) return;


    // Attribute class
    if ( PyType_Ready( coral::PyCoral::Attribute_Type() ) < 0 ) return;

    // AttributeListIterator class
    if ( PyType_Ready( coral::PyCoral::AttributeListIterator_Type() ) < 0 ) return;

    // ICursorIterator class
    if ( PyType_Ready( coral::PyCoral::ICursorIterator_Type() ) < 0 ) return;

    // ISessionProxy class
    if ( PyType_Ready( coral::PyCoral::ISessionProxy_Type() ) < 0 ) return;

    // ISchema class
    if ( PyType_Ready( coral::PyCoral::ISchema_Type() ) < 0 ) return;

    // IQueryDefinition class
    if ( PyType_Ready( coral::PyCoral::IQueryDefinition_Type() ) < 0 ) return;

    // IOperationWithQuery class
    if ( PyType_Ready( coral::PyCoral::IOperationWithQuery_Type() ) < 0 ) return;

    // IBulkOperation class
    if ( PyType_Ready( coral::PyCoral::IBulkOperation_Type() ) < 0 ) return;

    // IBulkOperationWithQuery class
    if ( PyType_Ready( coral::PyCoral::IBulkOperationWithQuery_Type() ) < 0 ) return;

    // ITable class
    if ( PyType_Ready( coral::PyCoral::ITable_Type() ) < 0 ) return;

    // ITableDataEditor class
    if ( PyType_Ready( coral::PyCoral::ITableDataEditor_Type() ) < 0 ) return;

    // ITableSchemaEditor class
    if ( PyType_Ready( coral::PyCoral::ITableSchemaEditor_Type() ) < 0 ) return;

    // ITableDescription class
    if ( PyType_Ready( coral::PyCoral::ITableDescription_Type() ) < 0 ) return;

    // TableDescription class
    PyTypeObject* TableDescription_Type = coral::PyCoral::TableDescription_Type();
    if ( PyType_Ready( TableDescription_Type ) < 0 ) return;

    // ITablePrivilegeManager class
    if ( PyType_Ready( coral::PyCoral::ITablePrivilegeManager_Type() ) < 0 ) return;

    // ISessionProperties class
    if ( PyType_Ready( coral::PyCoral::ISessionProperties_Type() ) < 0 ) return;

    // IPrimaryKey class
    if ( PyType_Ready( coral::PyCoral::IPrimaryKey_Type() ) < 0 ) return;

    // IForeignKey class
    if ( PyType_Ready( coral::PyCoral::IForeignKey_Type() ) < 0 ) return;

    // IUniqueConstraint class
    if ( PyType_Ready( coral::PyCoral::IUniqueConstraint_Type() ) < 0 ) return;

    // IIndex class
    if ( PyType_Ready( coral::PyCoral::IIndex_Type() ) < 0 ) return;

    // IColumn class
    if ( PyType_Ready( coral::PyCoral::IColumn_Type() ) < 0 ) return;

    // ITransaction class
    if ( PyType_Ready( coral::PyCoral::ITransaction_Type() ) < 0 ) return;

    // ITypeConverter class
    if ( PyType_Ready( coral::PyCoral::ITypeConverter_Type() ) < 0 ) return;


    // IQuery class
    if ( PyType_Ready( coral::PyCoral::IQuery_Type() ) < 0 ) return;

    // IWebCacheControl class
    if ( PyType_Ready( coral::PyCoral::IWebCacheControl_Type() ) < 0 ) return;

    // IWebCacheInfo class
    if ( PyType_Ready( coral::PyCoral::IWebCacheInfo_Type() ) < 0 ) return;

    // ICursor class
    if ( PyType_Ready( coral::PyCoral::ICursor_Type() ) < 0 ) return;

    // IView class
    if ( PyType_Ready( coral::PyCoral::IView_Type() ) < 0 ) return;

    // IViewFactory class
    if ( PyType_Ready( coral::PyCoral::IViewFactory_Type() ) < 0 ) return;

    // IMonitoringReporter class
    if ( PyType_Ready( coral::PyCoral::IMonitoringReporter_Type() ) < 0 ) return;

    // MessageStream class
    PyTypeObject* MessageStream_Type = coral::PyCoral::MessageStream_Type();
    if ( PyType_Ready( MessageStream_Type ) < 0 ) return;


    // Initialize the module
#ifdef WIN32
    PyObject* this_module = Py_InitModule3( (char*) "lcg_PyCoral", 0, (char*) "The Python Coral module" );
#else
    PyObject* this_module = Py_InitModule3( (char*) "liblcg_PyCoral", 0, (char*) "The Python Coral module" );
#endif

    // Initialize the exception
    PyObject* error = coral::PyCoral::Exception();
    Py_INCREF( error );
    PyModule_AddObject( this_module, (char*) "Exception", error );



    // Insert the classes into the module

    // Insert Context into the module
    //Py_INCREF( Context_Type );
    //    PyModule_AddObject( this_module, (char*) "Context", (PyObject*) Context_Type );

    // Insert ConnectionService into the module
    Py_INCREF( ConnectionService_Type );
    PyModule_AddObject( this_module, (char*) "ConnectionService", (PyObject*) ConnectionService_Type );

    // Insert Date into the module
    Py_INCREF( Date_Type );
    PyModule_AddObject( this_module, (char*) "Date", (PyObject*) Date_Type );

    // Insert TimeStamp into the module
    Py_INCREF( TimeStamp_Type );
    PyModule_AddObject( this_module, (char*) "TimeStamp", (PyObject*) TimeStamp_Type );

    // Insert Blob into the module
    Py_INCREF( Blob_Type );
    PyModule_AddObject( this_module, (char*) "Blob", (PyObject*) Blob_Type );

    // Insert AttributeList into the module
    Py_INCREF( AttributeList_Type );
    PyModule_AddObject( this_module, (char*) "AttributeList", (PyObject*) AttributeList_Type );

    // Insert TableDescription into the module
    Py_INCREF( TableDescription_Type );
    PyModule_AddObject( this_module, (char*) "TableDescription", (PyObject*) TableDescription_Type );

    // Insert MessageStream into the module
    Py_INCREF( MessageStream_Type );
    PyModule_AddObject( this_module, (char*) "MessageStream", (PyObject*) MessageStream_Type );



    // Insert the constants into the module

    // The Access mode constants: ReadOnly, Update
    PyModule_AddIntConstant( this_module, (char*) "access_ReadOnly", coral::ReadOnly );
    PyModule_AddIntConstant( this_module, (char*) "access_Update", coral::Update );

    // The Set Operation constants: Union, Minus, Intersect
    PyModule_AddIntConstant( this_module, (char*) "query_Union", coral::IQueryDefinition::Union );
    PyModule_AddIntConstant( this_module, (char*) "query_Minus", coral::IQueryDefinition::Minus );
    PyModule_AddIntConstant( this_module, (char*) "query_Intersect", coral::IQueryDefinition::Intersect );

    // The Privelege constants: Select, Update, Insert, Delete
    PyModule_AddIntConstant( this_module, (char*) "privilege_Select", coral::ITablePrivilegeManager::Select );
    PyModule_AddIntConstant( this_module, (char*) "privilege_Update", coral::ITablePrivilegeManager::Update );
    PyModule_AddIntConstant( this_module, (char*) "privilege_Insert", coral::ITablePrivilegeManager::Insert );
    PyModule_AddIntConstant( this_module, (char*) "privilege_Delete", coral::ITablePrivilegeManager::Delete );

    // The MonitoringReporter constants:
    PyModule_AddIntConstant( this_module, (char*) "monitor_Type_Info", coral::monitor::Info );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Type_Time", coral::monitor::Time );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Type_Warning", coral::monitor::Warning );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Type_Error", coral::monitor::Error );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Type_Config", coral::monitor::Config );

    // The MonitoringReporter constants:
    PyModule_AddIntConstant( this_module, (char*) "monitor_Level_Off", coral::monitor::Off );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Level_Minimal", coral::monitor::Minimal );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Level_Default", coral::monitor::Default );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Level_Debug", coral::monitor::Debug );
    PyModule_AddIntConstant( this_module, (char*) "monitor_Level_Trace", coral::monitor::Trace );

    // The MessageStream constants:
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Nil", coral::Nil );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Debug", coral::Debug );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Info", coral::Info );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Warning", coral::Warning );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Error", coral::Error );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Fatal", coral::Fatal );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Always", coral::Always );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_NumLevels", coral::NumLevels );
  }
}
