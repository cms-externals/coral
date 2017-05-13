// $Id: module.cpp,v 1.7 2010/03/19 23:35:03 avalassi Exp $
#include "MySQL_headers.h"

#include "CoralKernel/CoralPluginDef.h"

#include "Domain.h"

CORAL_PLUGIN_MODULE( "CORAL/RelationalPlugins/mysql", coral::MySQLAccess::Domain )
