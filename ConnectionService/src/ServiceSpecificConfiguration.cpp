#include "RelationalAccess/IDatabaseServiceDescription.h"
#include "ServiceSpecificConfiguration.h"
#include <sstream>

void
coral::ConnectionService::ServiceSpecificConfiguration::overWriteIfExistsSpecific(int& value, const std::string& paramName) const {
  std::map<std::string,std::string>::const_iterator iPar = m_serviceParams.find(paramName);
  if(iPar!=m_serviceParams.end()) {
    std::istringstream ss(iPar->second);
    ss >> value;
  }
}

int
coral::ConnectionService::ServiceSpecificConfiguration::connectionRetrialPeriod() const {
  int val = 0;
  if(m_configuration) {
    val = m_configuration->connectionRetrialPeriod();
    this->overWriteIfExistsSpecific(val,coral::IDatabaseServiceDescription::connectionRetrialPeriodParam());
  }
  return val;
}

int
coral::ConnectionService::ServiceSpecificConfiguration::connectionRetrialTimeOut() const {
  int val = 0;
  if(m_configuration) {
    val = m_configuration->connectionRetrialTimeOut();
    this->overWriteIfExistsSpecific(val,coral::IDatabaseServiceDescription::connectionRetrialTimeOutParam());
  }
  return val;
}

int
coral::ConnectionService::ServiceSpecificConfiguration::connectionTimeOut() const {
  int val = 0;
  if(m_configuration) {
    val = m_configuration->connectionTimeOut();
    // time-out=0 cannot be overwritten
    if(val) this->overWriteIfExistsSpecific(val,coral::IDatabaseServiceDescription::connectionTimeOutParam());
  }
  return val;
}

coral::ConnectionService::ServiceSpecificConfiguration::ServiceSpecificConfiguration() : m_serviceParams(),m_configuration(0),m_emptyString(""){
}

coral::ConnectionService::ServiceSpecificConfiguration::ServiceSpecificConfiguration(const ConnectionParams& params, const ConnectionServiceConfiguration& configuration) : m_serviceParams(params.serviceParameters()),m_configuration(&configuration),m_emptyString(""){
}

coral::ConnectionService::ServiceSpecificConfiguration::ServiceSpecificConfiguration(const ServiceSpecificConfiguration& rhs) :
  m_serviceParams(rhs.m_serviceParams),
  m_configuration(rhs.m_configuration),
  m_emptyString(""){
}

coral::ConnectionService::ServiceSpecificConfiguration&
coral::ConnectionService::ServiceSpecificConfiguration::operator=(const ServiceSpecificConfiguration& rhs){
  m_serviceParams = rhs.m_serviceParams;
  m_configuration = rhs.m_configuration;
  return *this;
}

bool
coral::ConnectionService::ServiceSpecificConfiguration::isMonitoringEnabled() const {
  bool ret = false;
  if(m_configuration) ret = m_configuration->isMonitoringEnabled();
  return ret;
}

coral::monitor::Level coral::ConnectionService::ServiceSpecificConfiguration::monitoringLevel() const {
  coral::monitor::Level lev = coral::monitor::Off;
  if(m_configuration) lev = m_configuration->monitoringLevel();
  return lev;
}

coral::Context*
coral::ConnectionService::ServiceSpecificConfiguration::serviceContext() const {
  coral::Context* ctx = 0;
  if(m_configuration) ctx = m_configuration->serviceContext();
  return ctx;
}

const std::string&
coral::ConnectionService::ServiceSpecificConfiguration::serviceName() const {
  if(!m_configuration) return m_emptyString;
  return m_configuration->serviceName();
}
