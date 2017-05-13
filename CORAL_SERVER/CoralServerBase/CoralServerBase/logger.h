// $Id: logger.h,v 1.5 2009/06/22 15:51:12 avalassi Exp $
#ifndef CORALSERVERBASE_LOGGER_H
#define CORALSERVERBASE_LOGGER_H 1

// Include files
#include "CoralBase/MessageStream.h"

// Logger source
#ifndef LOGGER_NAME
#error You must define macro LOGGER_NAME before including CoralServerBase/logger.h
#endif

// Logger definition
#define logger coral::MessageStream(LOGGER_NAME) << coral::Verbose
#define endlog coral::MessageStream::endmsg

#endif // CORALSERVERBASE_LOGGER_H
