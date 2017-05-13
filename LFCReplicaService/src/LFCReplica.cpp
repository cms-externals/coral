#include "LFCReplica.h"

#include "RelationalAccess/AccessMode.h"
#include "LFCPhysicalConnectionString.h"
#include "LFCCredentialString.h"
#include "ReplicaSet.h"
#include "ReplicaStatus.h"

using namespace coral::LFCReplicaService;

//-----------------------------------------------------------------------------

LFCReplica::LFCReplica(lfc_filereplica* replica)
  : fileid( replica->fileid )
  , sfn( replica->sfn )
  , status( replica->status == '1' )
  , host( replica->host )
  , f_type( replica->f_type )
  , poolname( replica->poolname )
  , fs( replica->fs )
{
}

//-----------------------------------------------------------------------------

LFCReplica::LFCReplica(const LFCReplica& rhs)
  : fileid(rhs.fileid)
  , sfn(rhs.sfn)
  , status(rhs.status)
  , host(rhs.host)
  , f_type(rhs.f_type)
  , poolname(rhs.poolname)
  , fs(rhs.fs)
{
}

//-----------------------------------------------------------------------------

LFCReplica::LFCReplica(const ReplicaDescription& rd, const std::string& guid)
  : fileid( 0 )
  , sfn( LFCPhysicalConnectionString::pfn( rd.physicalConnectionString(), rd.userName(), guid ) )
  , status( rd.status() == On )
  , host( rd.serverName() )
  , f_type( rd.accessMode() == coral::ReadOnly ? 'R' : 'W' )
  , poolname( rd.authenticationMechanism() )
  , fs( LFCCredentialString::credentials( rd.password(), guid) )
{
  if(poolname.empty()) poolname = "None";
}

//-----------------------------------------------------------------------------

//#include <iostream>

LFCReplica::~LFCReplica()
{
  //std::cout << "************ WHAT IS GOING ON? **********" << std::endl;
}

//-----------------------------------------------------------------------------

LFCReplicaSet::~LFCReplicaSet()
{
  for(std::vector<LFCReplica*>::iterator iR=m_replicas.begin(); iR!=m_replicas.end(); ++iR)
  {
    delete *iR;
  }
}

//-----------------------------------------------------------------------------
