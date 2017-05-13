#include "TestDriver.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
#include <iostream>

void
TestDriver::simpleAtLi()
{
  coral::AttributeList atlist;
  atlist.extend<double>( "x" );
  atlist.extend( "y", "float" );
  atlist.extend( "z", typeid(int) );
  atlist.extend( "d", typeid(coral::Date) );
  atlist.extend( "t", typeid(coral::TimeStamp) );
  atlist.extend( "ll", "long long" );

  atlist["y"].data<float>() = 2;
  atlist[2].data<int>() = 3;
  atlist.begin()->data<double>() = 1;
  atlist[3].data<coral::Date>() = coral::Date::today();
  atlist[4].setValue( coral::TimeStamp::now() );
  atlist[5].data<long long>() = 12345678900LL;

  atlist.toOutputStream( std::cout );
  std::cout << std::endl;
}


void
TestDriver::atLiWithSpec()
{
  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification;

  spec->extend<double>( "x" );
  spec->extend( "y", "float" );
  spec->extend( "z", typeid(int) );

  coral::AttributeList atlist1( *spec );
  atlist1.begin()->data<double>() = 1;
  atlist1["y"].data<float>() = 2;
  atlist1[2].data<int>() = 3;

  spec->extend<unsigned int>( "i" );
  coral::AttributeList atlist2( *spec );
  atlist2.begin()->data<double>() = 10;
  atlist2["y"].data<float>() = 20;
  atlist2[2].data<int>() = 30;
  atlist2[3].data<unsigned int>() = 40;

  spec->release();

  atlist1.toOutputStream( std::cout );
  std::cout << std::endl;

  atlist2.toOutputStream( std::cout );
  std::cout << std::endl;
}


void
TestDriver::atLiWithSharedSpec()
{
  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification;

  spec->extend<double>( "x" );
  spec->extend( "y", "float" );
  spec->extend( "z", typeid(int) );

  coral::AttributeList atlist1( *spec, true );
  atlist1.begin()->data<double>() = 1;
  atlist1["y"].data<float>() = 2;
  atlist1[2].data<int>() = 3;

  spec->extend<unsigned int>( "i" );
  coral::AttributeList atlist2( *spec, true );
  atlist2.begin()->data<double>() = 10;
  atlist2["y"].data<float>() = 20;
  atlist2[2].data<int>() = 30;
  atlist2[3].data<unsigned int>() = 40;

  spec->release();

  atlist1.toOutputStream( std::cout );
  std::cout << std::endl;

  atlist2.toOutputStream( std::cout );
  std::cout << std::endl;

  atlist2["y"].setNull();
  atlist2.toOutputStream( std::cout );
  std::cout << std::endl;

  coral::AttributeList atlist3 = atlist2;
  atlist3.toOutputStream( std::cout );
  std::cout << std::endl;

  coral::AttributeList atlist4 = atlist3;
  atlist4.extend< unsigned int >( "ii" );
  atlist4[ "ii" ].data< unsigned int >() = 44;
  atlist4.toOutputStream( std::cout );
  std::cout << std::endl;
}



void
TestDriver::atLiBound()
{
  class MyObject {
  public:
    MyObject() : x(1), y(2), z(3), b( true ) {}
    ~MyObject() {}
    double x;
    float y;
    long z;
    bool b;
  };

  MyObject myObject;

  coral::AttributeList atlist;
  atlist.extend<double>( "x" );
  atlist["x"].bind( myObject.x );

  atlist.extend<float>( "y" );
  atlist["y"].bind( myObject.y );

  atlist.extend<long>( "z" );
  atlist["z"].bindUnsafely( &(myObject.z) );

  atlist.extend<bool>( "b" );
  atlist["b"].bindUnsafely( &(myObject.b) );


  atlist.extend<std::string>( "unchanged" );
  atlist["unchanged"].data<std::string>() = "data";

  atlist.toOutputStream( std::cout );
  std::cout << std::endl;

  myObject.x = 123;
  myObject.y = 231;
  myObject.z = 312;
  myObject.b = false;

  atlist.toOutputStream( std::cout );
  std::cout << std::endl;

  atlist["x"].data<double>() = 0.123;

  std::cout << "Object.x = " << myObject.x << std::endl;
}


void
TestDriver::atLiShared()
{
  std::cout << "Testing shared lists..." << std::endl;

  coral::AttributeList atlist1;
  atlist1.extend<double>( "x" );
  atlist1["x"].data<double>() = 1;

  atlist1.extend<float>( "y" );
  atlist1["y"].data<float>() = 2;

  atlist1.extend<int>( "z" );
  atlist1["z"].data<int>() = 3;

  coral::AttributeList atlist2;
  atlist2.extend<double>( "XX" );
  atlist2["XX"].data<double>() = 10;

  atlist2.extend<float>( "YY" );
  // Share atlist2.YY with atlist1.y
  atlist1["y"].shareData( atlist2["YY"] );
  atlist1["y"].data<float>() = 2;

  atlist1.toOutputStream( std::cout ) << std::endl;
  atlist2.toOutputStream( std::cout ) << std::endl;

  coral::AttributeList atlist3 = atlist2;
  // Share attlist3.YY with atlist2.YY
  atlist3["YY"].shareData( atlist2["YY"] );
  atlist3["YY"].data<float>() = 123;

  atlist1.toOutputStream( std::cout ) << std::endl;
  atlist2.toOutputStream( std::cout ) << std::endl;
  atlist3.toOutputStream( std::cout ) << std::endl;

  std::cout << "Setting null" << std::endl;
  atlist1["y"].setNull( true );

  atlist1.toOutputStream( std::cout ) << std::endl;
  atlist2.toOutputStream( std::cout ) << std::endl;
  atlist3.toOutputStream( std::cout ) << std::endl;


  std::cout << "Setting not null" << std::endl;
  atlist3["YY"].setNull( false );

  atlist1.toOutputStream( std::cout ) << std::endl;
  atlist2.toOutputStream( std::cout ) << std::endl;
  atlist3.toOutputStream( std::cout ) << std::endl;
}
