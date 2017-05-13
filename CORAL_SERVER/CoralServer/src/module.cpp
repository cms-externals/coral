// $Id: module.cpp,v 1.4 2009/06/22 15:51:12 avalassi Exp $

// Include files
#include "CoralKernel/CoralPluginDef.h"

// Local include files
#include "CoralServerFacadeService.h"

//-----------------------------------------------------------------------------

CORAL_PLUGIN_MODULE( "CORAL/Services/CoralServerFacade",
                     coral::CoralServer::CoralServerFacadeService )

//-----------------------------------------------------------------------------
