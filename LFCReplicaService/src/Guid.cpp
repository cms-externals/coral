#include "Guid.h"
#include <uuid/uuid.h>
#include <cstring>
#include <cstdio>

// Function definitions of Guid class.  These functions are taken from pool->LFCCatalogue for GUID generation

static const char* fmt_Guid ="%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX";

void coral::LFCReplicaService::Guid::create(Guid& guid)
{
  uuid_t me_;
  ::uuid_generate_time(me_);
  unsigned int  *d1=(unsigned int*)me_;
  unsigned short *d2=(unsigned short*)(me_+4);
  unsigned short *d3=(unsigned short*)(me_+6);
  guid.Data1 = *d1;
  guid.Data2 = *d2;
  guid.Data3 = *d3;
  for (int i=0; i<8; i++) {
    guid.Data4[i]=me_[i+8];
  }
}

std::string coral::LFCReplicaService::Guid::toString() const
{
  char text[128];
  ::sprintf(text, fmt_Guid,
            Data1, Data2, Data3,
            Data4[0], Data4[1], Data4[2], Data4[3],
            Data4[4], Data4[5], Data4[6], Data4[7]);
  return text;
}

// Automatic conversion to string representation
const coral::LFCReplicaService::Guid& coral::LFCReplicaService::Guid::fromString(const std::string& source)
{
  unsigned char d[8];
  ::sscanf( source.c_str(), fmt_Guid, &Data1, &Data2, &Data3,
            &Data4[0], &Data4[1], &Data4[2], &Data4[3], &d[0], &d[1], &d[2], &d[3]);
  //*(int*)&Data4[4] = *(int*)d;
  *(unsigned char*)&Data4[4] = *(unsigned char*)d; // Fix gcc46 warning (bug #89768)
  return *this;
}

bool coral::LFCReplicaService::Guid::operator < (const Guid& g)  const
{
  return ::memcmp(&g.Data1, &Data1, 16) < 0;
}

coral::LFCReplicaService::Guid& coral::LFCReplicaService::Guid::operator=(const Guid& g)
{
  if ( this != &g )
  {
    Data1 = g.Data1;
    Data2 = g.Data2;
    Data3 = g.Data3;
    unsigned int *p = (unsigned int*)&Data4[0];
    const unsigned int *q = (const unsigned int*)&g.Data4[0];
    *(p+1) = *(q+1);
    *p = *q;
  }
  return *this;
}
bool coral::LFCReplicaService::Guid::operator==(const Guid& g)  const
{
  if ( this != & g ) {
    if (Data1 != g.Data1 ) return false;
    if (Data2 != g.Data2 ) return false;
    if (Data3 != g.Data3 ) return false;
    const unsigned int *p = (const unsigned int*)&Data4[0],*q = (const unsigned int*)&g.Data4[0];
    return *p++ == *q++ && *p == *q;
  }
  return true;
}

bool coral::LFCReplicaService::Guid::operator!=(const Guid& g)  const
{
  return !(this->operator==(g));
}

bool coral::LFCReplicaService::Guid::isGuid(const std::string& s){
  std::string hexCharSet("0123456789abcdefABCDEF");
  bool ok = true;
  if(s.size()!=36) ok = false;
  if(ok) {
    size_t j=0;
    size_t separPos[4]={8,13,18,23};
    for(size_t i=0; i<s.size(); i++) {
      // first check that separators '-' are 4 and  in the expected positions
      if(s[i]=='-') {
        if(j>3) {
          ok=false;
          break;
        }
        if(i==separPos[j]) {
          j++;
        } else {
          ok = false;
          break;
        }
      } else {
        // all the remaining chars must be in the hexCharSet
        if(hexCharSet.find(s[i])==std::string::npos) {
          ok = false;
          break;
        }
      }
    }
  }
  return ok;
}
