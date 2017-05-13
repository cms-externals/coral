#include <stdexcept>

#include "CoralBase/MessageStream.h"
#include "CoralBase/TimeStamp.h"

#include "RelationalAccess/ConnectionServiceException.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "RelationalAccess/IDatabaseServiceSet.h"
#include "RelationalAccess/ILookupService.h"
#include "RelationalAccess/IRelationalDomain.h"
#include "RelationalAccess/IRelationalService.h"
#include "RelationalAccess/IReplicaSortingAlgorithm.h"

#include "ConnectionServiceConfiguration.h"
#include "ConnectionString.h"
#include "ReplicaCatalogue.h"

typedef coral::ConnectionService::ReplicaCatalogue::PosixTime CsPosixTime;

/// constructor
coral::ConnectionService::ReplicaCatalogue::ReplicaCatalogue( const ConnectionServiceConfiguration& serviceConfiguration ) :
  m_serviceConfiguration( serviceConfiguration ),
  m_excludedConnections(),
  m_mutexForExclusionList(){
}

/// destructor
coral::ConnectionService::ReplicaCatalogue::~ReplicaCatalogue(){
}

/// sets the specified connection as unavailable
void
coral::ConnectionService::ReplicaCatalogue::setConnectionExcluded( const std::string& technologyName,
                                                                   const std::string& serviceName )
{
  coral::MessageStream log( m_serviceConfiguration.serviceName() );

  {
    boost::mutex::scoped_lock lock(m_mutexForExclusionList);
    std::pair<std::map<std::string,CsPosixTime>::iterator,bool> ret =
      m_excludedConnections.insert(
                                   std::make_pair(ConnectionString::serviceKey(technologyName,serviceName), coral::TimeStamp::now(true).time() ) );
    if(!ret.second) ret.first->second = coral::TimeStamp::now(true).time();
  }

  log << coral::Warning << "Service " << serviceName << " (technology=\""<<technologyName<<"\") is currently unavailable, and will be temporarely excluded from replicas." << coral::MessageStream::endmsg;
}

/// resolves the technology specified in the connection string, checking if it is supported by an
/// existing plug-in...
bool
coral::ConnectionService::ReplicaCatalogue::isConnectionPhysical( const std::string& connectionString ) const {
  bool physical = false;
  coral::IRelationalService& relationalService = m_serviceConfiguration.relationalService();

  std::vector<std::string> technologies = relationalService.availableTechnologies();
  std::vector<std::string>::iterator iter=technologies.begin();

  while( (iter != technologies.end()) && (physical == false))
  {
    std::string::size_type pos = connectionString.find(*iter);
    if (pos != std::string::npos) {
      physical = true;
    }
    iter++;
  }
  return physical;
}

/// resolves the technology specified in the connection string, checking if it is supported by an
/// existing plug-in...
bool
coral::ConnectionService::ReplicaCatalogue::isConnectionExcluded( const std::string& technologyName,
                                                                  const std::string& serviceName ) const {
  boost::mutex::scoped_lock lock(m_mutexForExclusionList);
  bool excluded = false;
  std::map<std::string,CsPosixTime>::iterator iConn =
    m_excludedConnections.find( ConnectionString::serviceKey(technologyName,serviceName ));
  if(iConn!=m_excludedConnections.end()) {
    coral::MessageStream log( m_serviceConfiguration.serviceName() );

    excluded = (( coral::TimeStamp::now(true).time().time_of_day().seconds()-iConn->second.time_of_day().seconds()) < m_serviceConfiguration.missingConnectionExclusionTime() );
    if(!excluded) {
      m_excludedConnections.erase(iConn);
      log << coral::Debug << "Service " << serviceName << " (technology=\""<<technologyName<<"\") has been re-admitted in the list of replicas." << coral::MessageStream::endmsg;
    } else {
      log << coral::Info << "Service " << serviceName << " (technology=\""<<technologyName<<"\") is currently excluded from replicas." << coral::MessageStream::endmsg;
    }
  }
  return excluded;
}

/// returns an array of connection strings - the replicas of the specified connection
std::vector<coral::ConnectionService::ConnectionParams>*
coral::ConnectionService::ReplicaCatalogue::getReplicas( const std::string& connectionString,
                                                         AccessMode accessMode ) const
{
  coral::MessageStream log( m_serviceConfiguration.serviceName() );
  std::vector<ConnectionParams>*  replicas = new std::vector<ConnectionParams>;
  if(isConnectionPhysical( connectionString )) {
    replicas->push_back( ConnectionParams(connectionString) );
  } else {
    log << coral::Debug << "Looking up "<<
      ((accessMode==coral::ReadOnly) ? std::string("read-only") : std::string("update") )<<
      " replicas for logical connection: \""<< connectionString<<"\""<< coral::MessageStream::endmsg;
    coral::ILookupService& lookupService = m_serviceConfiguration.lookupService();

    coral::IDatabaseServiceSet* dbSet = lookupService.lookup( connectionString, accessMode );
    int numOfReplicas = dbSet->numberOfReplicas();
    log << coral::Debug << "Lookup service found " << numOfReplicas << " replicas" << coral::MessageStream::endmsg;
    std::vector<const coral::IDatabaseServiceDescription*> sortedReplicaSet;
    for(int j=0; j<numOfReplicas; j++) {
      sortedReplicaSet.push_back(&dbSet->replica( j ));
    }
    coral::IReplicaSortingAlgorithm* sortingAlgorithm = m_serviceConfiguration.sortingAlgorithm();
    if(sortingAlgorithm) {
      log << coral::Info << "Re-ordering " << sortedReplicaSet.size() << " replicas using installed algorithm" << coral::MessageStream::endmsg;
      sortingAlgorithm->sort(sortedReplicaSet);
    }
    numOfReplicas = sortedReplicaSet.size();
    if(sortingAlgorithm)
      log << coral::Info << "Sorting algorithm selected " << numOfReplicas << " replicas" << coral::MessageStream::endmsg;
    if( numOfReplicas>0 ) {
      // set the number of replicas to 1 in update mode or if failover is not enabled
      if((accessMode == coral::Update) || !m_serviceConfiguration.isReplicaFailoverEnabled()) {
        numOfReplicas = 1;
        if( accessMode == coral::Update )
          log << coral::Debug << "Database will be accessed in update mode: keep only 1 replica" << coral::MessageStream::endmsg;
        else
          log << coral::Debug << "Failover is disabled: keep only 1 replica" << coral::MessageStream::endmsg;
      }
    }
    for(int i=0; i<numOfReplicas; i++) {
      std::string replicaConnectionString = sortedReplicaSet[ i ]->connectionString();
      if(replicaConnectionString.empty()) continue;

      bool readOnlyOnUpdate = m_serviceConfiguration.isReadOnlySessionOnUpdateConnectionsEnabled();
      if((accessMode==coral::Update) || // update capable connection are selected by the lookup
         readOnlyOnUpdate || // for read-only connection the lookup will also consider the update
         (dbSet->replica( i ).accessMode()==coral::ReadOnly)) // otherwise strictly select the read-only capable connection
      {
        ConnectionParams connPars(replicaConnectionString);
        // loading service-specific parameters
        std::string connRetrPeriod =
          sortedReplicaSet[ i ]->serviceParameter(coral::IDatabaseServiceDescription::connectionRetrialPeriodParam());
        if(!connRetrPeriod.empty())
          connPars.serviceParameters().insert(
                                              std::make_pair(coral::IDatabaseServiceDescription::connectionRetrialPeriodParam(),connRetrPeriod));
        std::string connRetrTime =
          sortedReplicaSet[ i ]->serviceParameter(coral::IDatabaseServiceDescription::connectionRetrialTimeOutParam());
        if(!connRetrTime.empty())
          connPars.serviceParameters().insert(
                                              std::make_pair(coral::IDatabaseServiceDescription::connectionRetrialTimeOutParam(),connRetrTime));

        std::string connTime =
          sortedReplicaSet[ i ]->serviceParameter(coral::IDatabaseServiceDescription::connectionTimeOutParam());
        if(!connTime.empty())
          connPars.serviceParameters().insert(
                                              std::make_pair(coral::IDatabaseServiceDescription::connectionTimeOutParam(),connTime));
        replicas->push_back( connPars );
      }
    }
    delete dbSet;
    if( replicas->empty() ) {
      throw coral::ReplicaNotAvailableException( connectionString ,accessMode,"ReplicaCatalogue::replicasForConnection");
    }
  }
  return replicas;
}
