#ifndef CORAL_CORALSTUBS_CALPROTOCOL_H
#define CORAL_CORALSTUBS_CALPROTOCOL_H

//CoralServerBase includes
#include "CoralServerBase/CALOpcode.h"

namespace coral {

  namespace CALOpcodes {

    static const CALOpcode FetchRows              = 0x10;
    static const CALOpcode FetchRowsNext          = 0x11;
    static const CALOpcode ReleaseCursor          = 0x12;
    static const CALOpcode FetchAllRows           = 0x13;
    static const CALOpcode ListTables             = 0x14;
    static const CALOpcode FetchTableDescription  = 0x15;
    static const CALOpcode TableExists            = 0x16;
    static const CALOpcode FetchSessionProperties = 0x17;
    static const CALOpcode FetchRowsOT            = 0x18;
    static const CALOpcode FetchAllRowsOT         = 0x19;
    static const CALOpcode ListViews              = 0x1a;
    static const CALOpcode FetchViewDescription   = 0x1b;
    static const CALOpcode ViewExists             = 0x1c;

  }

}

#endif
