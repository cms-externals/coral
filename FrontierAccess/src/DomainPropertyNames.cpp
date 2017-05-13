#include "DomainPropertyNames.h"

std::string
coral::FrontierAccess::DomainPropertyNames::tableSpaceForTables()
{
  static const std::string s_tableSpaceForTables = "TableSpaceForTables";
  return s_tableSpaceForTables;
}

std::string
coral::FrontierAccess::DomainPropertyNames::tableSpaceForIndices()
{
  static const std::string s_tableSpaceForIndices = "TableSpaceForIndices";
  return s_tableSpaceForIndices;
}

std::string
coral::FrontierAccess::DomainPropertyNames::tableSpaceForLobs()
{
  static const std::string s_tableSpaceForLobs = "TableSpaceForLobs";
  return s_tableSpaceForLobs;
}

std::string
coral::FrontierAccess::DomainPropertyNames::lobChunkSize()
{
  static const std::string s_lobChunkSize = "LobChunkSize";
  return s_lobChunkSize;
}
