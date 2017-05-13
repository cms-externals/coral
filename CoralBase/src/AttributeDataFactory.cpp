#include "AttributeDataFactory.h"
#include "AttributeTemplatedData.h"
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include <string>

const coral::AttributeDataFactory&
coral::AttributeDataFactory::factory()
{
  static const coral::AttributeDataFactory s_factory;
  return s_factory;
}


coral::AttributeData*
coral::AttributeDataFactory::create( const std::type_info& type ) const
{
  if ( type == typeid(bool) ) return new AttributeTemplatedData<bool>();
  else if ( type == typeid(char) ) return new AttributeTemplatedData<char>();
  else if ( type == typeid(unsigned char) ) return new AttributeTemplatedData<unsigned char>();
  else if ( type == typeid(short) ) return new AttributeTemplatedData<short>();
  else if ( type == typeid(unsigned short) ) return new AttributeTemplatedData<unsigned short>();
  else if ( type == typeid(int) ) return new AttributeTemplatedData<int>();
  else if ( type == typeid(unsigned int) ) return new AttributeTemplatedData<unsigned int>();
  else if ( type == typeid(long) ) return new AttributeTemplatedData<long>();
  else if ( type == typeid(unsigned long) ) return new AttributeTemplatedData<unsigned long>();
  else if ( type == typeid(long long) ) return new AttributeTemplatedData<long long>();
  else if ( type == typeid(unsigned long long) ) return new AttributeTemplatedData<unsigned long long>();
  else if ( type == typeid(float) ) return new AttributeTemplatedData<float>();
  else if ( type == typeid(double) ) return new AttributeTemplatedData<double>();
  else if ( type == typeid(long double) ) return new AttributeTemplatedData<long double>();
  else if ( type == typeid(std::string) ) return new AttributeTemplatedData<std::string>();
  else if ( type == typeid(coral::Blob) ) return new AttributeTemplatedData<coral::Blob>();
  else if ( type == typeid(coral::Date) ) return new AttributeTemplatedData<coral::Date>();
  else if ( type == typeid(coral::TimeStamp) ) return new AttributeTemplatedData<coral::TimeStamp>();
  else return 0;
}
