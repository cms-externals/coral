// $Id: MySQL_headers.h,v 1.3 2011/03/22 10:29:54 avalassi Exp $
#ifndef MYSQLACCESS_MYSQL_HEADERS_H
#define MYSQLACCESS_MYSQL_HEADERS_H 1

#ifdef _WIN32
#define NOGDI
#define __LCC__
#endif
// On WIN32, pulls in winsock.h or winsock2.h if MYSQL_VERSION_ID < or >= 60000
#include "mysql.h"

#endif
