#include "MysqlSchema.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/ITransaction.h"
#include <iostream>




MysqlSchema::MysqlSchema(coral::ISessionProxy *proxy) :
  m_proxy (proxy)
{
}


MysqlSchema::~MysqlSchema()
{

}
