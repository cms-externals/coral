#ifdef _DEBUG
#undef _DEBUG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#include "Python.h"
#pragma clang diagnostic pop
#define _DEBUG
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"
#include "Python.h"
#pragma clang diagnostic pop
#endif
#include "Exception.h"
#include "PyCoral/Date.h"
#include "PyCoral/TimeStamp.h"
#include "PyCoral/Blob.h"
#include "ConnectionService.h"
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
#include "TableDescription.h"
#include "ITablePrivilegeManager.h"
#include "ISessionProperties.h"
#include "IOperationWithQuery.h"
#include "IBulkOperation.h"
#include "IBulkOperationWithQuery.h"
#include "IWebCacheControl.h"
#include "IWebCacheInfo.h"
#include "IMonitoringReporter.h"
#include "PyCoral/Attribute.h"
#include "PyCoral/AttributeList.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "RelationalAccess/IMonitoring.h"
#include "AttributeSpecification.h"
#include "AttributeListIterator.h"
#include "ICursorIterator.h"
#include "MessageStream.h"
#include "CoralBase/MessageStream.h"
#include <iostream>
// Module entry point
extern "C" {
static PyObject *
liblcg_PyCoralinit(void)
  {

    static PyObject *fake_module;
    // Check on the classes

    // Context class
    //    PyTypeObject* Context_Type = coral::PyCoral::Context_Type();
    //    if ( PyType_Ready( Context_Type ) < 0 ) return fake_module;

    // ConnectionService class
    PyTypeObject* ConnectionService_Type = coral::PyCoral::ConnectionService_Type();
    if ( PyType_Ready( ConnectionService_Type ) < 0 ) return fake_module;

    // IConnectionServiceConfiguration class
    if ( PyType_Ready( coral::PyCoral::IConnectionServiceConfiguration_Type() ) < 0 ) return fake_module;

    // Date class
    PyTypeObject* Date_Type = coral::PyCoral::Date_Type();
    if ( PyType_Ready( Date_Type ) < 0 ) return fake_module;

    // TimeStamp class
    PyTypeObject* TimeStamp_Type = coral::PyCoral::TimeStamp_Type();
    if ( PyType_Ready( TimeStamp_Type ) < 0 ) return fake_module;

    // Blob class
    PyTypeObject* Blob_Type = coral::PyCoral::Blob_Type();
    if ( PyType_Ready( Blob_Type ) < 0 ) return fake_module;

    // AttributeSpecification class
    if ( PyType_Ready( coral::PyCoral::AttributeSpecification_Type() ) < 0 ) return fake_module;

    // AttributeList class
    PyTypeObject* AttributeList_Type = coral::PyCoral::AttributeList_Type();
    if ( PyType_Ready( AttributeList_Type ) < 0 ) return fake_module;


    // Attribute class
    if ( PyType_Ready( coral::PyCoral::Attribute_Type() ) < 0 ) return fake_module;

    // AttributeListIterator class
    if ( PyType_Ready( coral::PyCoral::AttributeListIterator_Type() ) < 0 ) return fake_module;

    // ICursorIterator class
    if ( PyType_Ready( coral::PyCoral::ICursorIterator_Type() ) < 0 ) return fake_module;

    // ISessionProxy class
    if ( PyType_Ready( coral::PyCoral::ISessionProxy_Type() ) < 0 ) return fake_module;

    // ISchema class
    if ( PyType_Ready( coral::PyCoral::ISchema_Type() ) < 0 ) return fake_module;

    // IQueryDefinition class
    if ( PyType_Ready( coral::PyCoral::IQueryDefinition_Type() ) < 0 ) return fake_module;

    // IOperationWithQuery class
    if ( PyType_Ready( coral::PyCoral::IOperationWithQuery_Type() ) < 0 ) return fake_module;

    // IBulkOperation class
    if ( PyType_Ready( coral::PyCoral::IBulkOperation_Type() ) < 0 ) return fake_module;

    // IBulkOperationWithQuery class
    if ( PyType_Ready( coral::PyCoral::IBulkOperationWithQuery_Type() ) < 0 ) return fake_module;

    // ITable class
    if ( PyType_Ready( coral::PyCoral::ITable_Type() ) < 0 ) return fake_module;

    // ITableDataEditor class
    if ( PyType_Ready( coral::PyCoral::ITableDataEditor_Type() ) < 0 ) return fake_module;

    // ITableSchemaEditor class
    if ( PyType_Ready( coral::PyCoral::ITableSchemaEditor_Type() ) < 0 ) return fake_module;

    // ITableDescription class
    if ( PyType_Ready( coral::PyCoral::ITableDescription_Type() ) < 0 ) return fake_module;

    // TableDescription class
    PyTypeObject* TableDescription_Type = coral::PyCoral::TableDescription_Type();
    if ( PyType_Ready( TableDescription_Type ) < 0 ) return fake_module;

    // ITablePrivilegeManager class
    if ( PyType_Ready( coral::PyCoral::ITablePrivilegeManager_Type() ) < 0 ) return fake_module;

    // ISessionProperties class
    if ( PyType_Ready( coral::PyCoral::ISessionProperties_Type() ) < 0 ) return fake_module;

    // IPrimaryKey class
    if ( PyType_Ready( coral::PyCoral::IPrimaryKey_Type() ) < 0 ) return fake_module;

    // IForeignKey class
    if ( PyType_Ready( coral::PyCoral::IForeignKey_Type() ) < 0 ) return fake_module;

    // IUniqueConstraint class
    if ( PyType_Ready( coral::PyCoral::IUniqueConstraint_Type() ) < 0 ) return fake_module;

    // IIndex class
    if ( PyType_Ready( coral::PyCoral::IIndex_Type() ) < 0 ) return fake_module;

    // IColumn class
    if ( PyType_Ready( coral::PyCoral::IColumn_Type() ) < 0 ) return fake_module;

    // ITransaction class
    if ( PyType_Ready( coral::PyCoral::ITransaction_Type() ) < 0 ) return fake_module;

    // ITypeConverter class
    if ( PyType_Ready( coral::PyCoral::ITypeConverter_Type() ) < 0 ) return fake_module;


    // IQuery class
    if ( PyType_Ready( coral::PyCoral::IQuery_Type() ) < 0 ) return fake_module;

    // IWebCacheControl class
    if ( PyType_Ready( coral::PyCoral::IWebCacheControl_Type() ) < 0 ) return fake_module;

    // IWebCacheInfo class
    if ( PyType_Ready( coral::PyCoral::IWebCacheInfo_Type() ) < 0 ) return fake_module;

    // ICursor class
    if ( PyType_Ready( coral::PyCoral::ICursor_Type() ) < 0 ) return fake_module;

    // IView class
    if ( PyType_Ready( coral::PyCoral::IView_Type() ) < 0 ) return fake_module;

    // IViewFactory class
    if ( PyType_Ready( coral::PyCoral::IViewFactory_Type() ) < 0 ) return fake_module;

    // IMonitoringReporter class
    if ( PyType_Ready( coral::PyCoral::IMonitoringReporter_Type() ) < 0 ) return fake_module;

    // MessageStream class
    PyTypeObject* MessageStream_Type = coral::PyCoral::MessageStream_Type();
    if ( PyType_Ready( MessageStream_Type ) < 0 ) return fake_module;


    // Initialize the module
    #if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "liblcg_PyCoral",     /* m_name */
        "The Python Coral module",  /* m_doc */
        -1,                  /* m_size */
        NULL,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
    PyObject* this_module = PyModule_Create(&moduledef);
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
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Verbose", coral::Verbose ); // fix bug #96544
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Debug", coral::Debug );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Info", coral::Info );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Warning", coral::Warning );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Error", coral::Error );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Fatal", coral::Fatal );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_Always", coral::Always );
    PyModule_AddIntConstant( this_module, (char*) "message_Level_NumLevels", coral::NumLevels );
    return this_module;
  }

#if PY_MAJOR_VERSION >= 3   
    PyMODINIT_FUNC PyInit_liblcg_PyCoral(void)
    {
        return liblcg_PyCoralinit();
    }
#else
    PyMODINIT_FUNC initliblcg_PyCoral(){
        liblcg_PyCoralinit();
    }
#endif
}
