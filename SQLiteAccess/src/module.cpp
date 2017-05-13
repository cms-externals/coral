#include <iostream>
#include "Domain.h"
#include "CoralKernel/CoralPluginDef.h"

CORAL_PLUGIN_MODULE( "CORAL/RelationalPlugins/sqlite", coral::SQLiteAccess::Domain )
