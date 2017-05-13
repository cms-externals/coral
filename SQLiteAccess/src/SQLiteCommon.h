#ifndef SQLITEACCESS_SQLITECOMMON_H
#define SQLITEACCESS_SQLITECOMMON_H
#include <typeinfo>
#include "CoralBase/Blob.h"
#include "CoralBase/Date.h"
#include "CoralBase/TimeStamp.h"
/**@file SQLiteCommon.h
 *This header provides collection of common definitions and helper functors
 @author Zhen Xie
 @date 03/09/2005
*/
namespace coral {
  namespace SQLiteAccess {
    enum SQLT {SQLT_INT=0,SQLT_UIN,SQLT_64INT,SQLT_64UIN,SQLT_DOUBLE,SQLT_TEXT,SQLT_BLOB,SQLT_DATE,SQLT_TIMESTAMP,SQLT_NULL};
    struct SQLiteType {
      SQLT operator()( const std::type_info& t ) const {
        if( t==typeid(int)||t==typeid(short)
            || (t==typeid(long) && sizeof(long) < sizeof(long long))
            ||t==typeid(char)) return SQLT_INT;
        if( t==typeid(unsigned short) ||t==typeid(bool)||t==typeid(unsigned char) ) return SQLT_UIN;
        if( (t==typeid(long) && sizeof(long) == sizeof(long long))
            ||t==typeid(long long int)) return SQLT_64INT;
        if( t==typeid(unsigned long) || t==typeid(unsigned int)
            ||t==typeid(unsigned long long int)) return SQLT_64UIN;
        if(t==typeid(float)||t==typeid(double)||t==typeid(long double)) return SQLT_DOUBLE;
        if(t==typeid(std::string)) return SQLT_TEXT;
        if(t==typeid(coral::Blob)) return SQLT_BLOB;
        if(t==typeid(coral::Date)) return SQLT_DATE;
        if(t==typeid(coral::TimeStamp)) return SQLT_TIMESTAMP;
        return SQLT_NULL;
      }
    };
    struct castAttributeToInt {
      int operator()(void* addr, const std::type_info&t){
        if(t==typeid(int)) {
          int* rs=reinterpret_cast<int*>(addr);
          return *rs;
        }
        if(t==typeid(short)) {
          short* rs=reinterpret_cast<short*>(addr);
          return (int)(*rs);
        }
        if(t==typeid(long)) {
          long* rs=reinterpret_cast<long*>(addr);
          return (int)*rs;
        }
        if(t==typeid(char)) {
          char* rs=reinterpret_cast<char*>(addr);
          return (int)*rs;
        }
        int* rs=reinterpret_cast<int*>(addr);
        return *rs;
      }
    };
    struct castAttributeToUInt {
      unsigned int operator()(void* addr, const std::type_info&t){
        if(t==typeid(unsigned int)) {
          unsigned int* rs=reinterpret_cast<unsigned int*>(addr);
          return (unsigned int)(*rs);
        }
        if(t==typeid(unsigned short)) {
          unsigned short* rs=reinterpret_cast<unsigned short*>(addr);
          return (unsigned int)(*rs);
        }
        if(t==typeid(unsigned long)) {
          unsigned long* rs=reinterpret_cast<unsigned long*>(addr);
          return (unsigned int)(*rs);
        }
        if(t==typeid(unsigned char)) {
          unsigned char* rs=reinterpret_cast<unsigned char*>(addr);
          return (unsigned int)(*rs);
        }
        if(t==typeid(bool)) {
          bool* rs=reinterpret_cast<bool*>(addr);
          return (unsigned int)(*rs);
        }
        unsigned int* rs=reinterpret_cast<unsigned int*>(addr);
        return *rs;
      }
    };
    struct castAttributeTo64UInt {
      unsigned long long int operator()(void* addr, const std::type_info&t){
        if(t==typeid(unsigned int)) {
          unsigned int* rs=reinterpret_cast<unsigned int*>(addr);
          return *rs;
        }

        if(t==typeid(unsigned long)) {
          unsigned long* rs=reinterpret_cast<unsigned long*>(addr);
          return *rs;
        }

        if(t==typeid(unsigned long long int)) {
          unsigned long long int* rs=reinterpret_cast<unsigned long long int*>(addr);
          return *rs;
        }
        if(t==typeid(unsigned long long)) {
          unsigned long long* rs=reinterpret_cast<unsigned long long*>(addr);
          return (unsigned long long int)(*rs);
        }
        unsigned long long int* rs=reinterpret_cast<unsigned long long int*>(addr);
        return *rs;
      }
    };
    struct castAttributeTo64Int {
      long long int operator()(void* addr, const std::type_info&t){
        if(t==typeid(unsigned int)) {
          unsigned int* rs=reinterpret_cast<unsigned int*>(addr);
          return *rs;
        }

        if(t==typeid(unsigned long)) {
          unsigned long* rs=reinterpret_cast<unsigned long*>(addr);
          return *rs;
        }

        if(t==typeid(unsigned long long int)) {
          unsigned long long int* rs=reinterpret_cast<unsigned long long int*>(addr);
          return *rs;
        }
        if(t==typeid(long long)) {
          long long int* rs=reinterpret_cast<long long int*>(addr);
          return (long long int)(*rs);
        }
        long long int* rs=reinterpret_cast<long long int*>(addr);
        return *rs;
      }
    };
    struct castAttributeToDouble {
      double operator()(void* addr, const std::type_info&t){
        if(t==typeid(double)) {
          double* rs=reinterpret_cast<double*>(addr);
          return *rs;
        }
        if(t==typeid(float)) {
          float* rs=reinterpret_cast<float*>(addr);
          return (double)(*rs);
        }
        if(t==typeid(long double)) {
          long double* rs=reinterpret_cast<long double*>(addr);
          return (double)(*rs); //losing 4 bytes from cast if long double is bigger than double
        }
        double* rs=reinterpret_cast<double*>(addr);
        return *rs;
      }
    };
  } //ns SQLiteAccess
} //ns pool
#endif /* SQLITEACCESS_SQLITECOMMON_H */
