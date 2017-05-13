#ifndef TESTING_CONFIG_H
#define TESTING_CONFIG_H

#include <map>
#include <set>
#include <string>

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"

#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/IAuthenticationService.h"
#include "RelationalAccess/IAuthenticationCredentials.h"
#include "RelationalAccess/IBulkOperation.h"
#include "RelationalAccess/IBulkOperationWithQuery.h"
#include "RelationalAccess/IColumn.h"
#include "RelationalAccess/IConnection.h"
#include "RelationalAccess/IConnectionService.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/IForeignKey.h"
#include "RelationalAccess/IIndex.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IMonitoringReporter.h"
#include "RelationalAccess/IMonitoringService.h"
#include "RelationalAccess/IOperationWithQuery.h"
#include "RelationalAccess/IPrimaryKey.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/IQueryDefinition.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ISession.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ITableDataEditor.h"
#include "RelationalAccess/ITablePrivilegeManager.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IUniqueConstraint.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/IViewFactory.h"
#include "RelationalAccess/TableDescription.h"

#include "TestEnv/TestEnv.h"

#endif
