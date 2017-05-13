// $Id: SetField.h,v 1.8 2011/03/22 10:29:54 avalassi Exp $
#ifndef CORAL_MYSQLACCESS_SETFIELD_H
#define CORAL_MYSQLACCESS_SETFIELD_H 1

namespace coral {
  namespace MySQLAccess {

    template <class T, int version> void setField( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      T value; std::istringstream is( buffer ); is>>value; attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    // AV 11-01-2006 wrong value was retrieved (only 1st of 3 digits used)
    template <> void setField<bool,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      bool value = ( atoi(buffer) == 1 ) ? true : false; attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> void setField<unsigned char,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      unsigned char value = atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> void setField<char,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      char value = atoi(buffer); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> void setField<std::string,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      std::string value( buffer ); attr.setValueFromAddress( (void*)(&value) ); attr.setNull( false );
    }

    template <> void setField<coral::Date,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      std::string dbuf = buffer;

      int year  = atoi( dbuf.substr( 0, 4 ).c_str() );
      int month = atoi( dbuf.substr( 5, 2 ).c_str() );
      int day   = atoi( dbuf.substr( 8    ).c_str() );

      coral::Date value( year, month, day );

      attr.setValueFromAddress( (void*)(&value) );
      attr.setNull( false );
    }

    template <> void setField<coral::TimeStamp,41>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      // MySQL 4.1.x and later, 19 characters long time stamp format: 2006-02-01 14:14:03
      //                1111111111
      //      01234567890123456789
      // e.g. YYYY-MM-DD hh:mm:ss
      std::string tsbuf = buffer;
      coral::TimeStamp value;

      int year  = atoi( tsbuf.substr(  0, 4 ).c_str() ); int month = atoi( tsbuf.substr(  5, 2 ).c_str() ); int day   = atoi( tsbuf.substr(  8, 2 ).c_str() );
      int hour  = atoi( tsbuf.substr(  11, 2 ).c_str() ); int mins  = atoi( tsbuf.substr( 14, 2 ).c_str() ); int secs  = atoi( tsbuf.substr( 17    ).c_str() );
      //std::cout << "YYYY: " << year << " MM: " << month << " DD: " << day << " HH: " << hour << " MM: " << mins << " SS: " << secs << std::endl;

      // Replace the value with the data from result set column
      value = coral::TimeStamp( year, month, day, hour, mins, secs, 0 );
      attr.setValueFromAddress( (void*)(&value) );
      attr.setNull( false );
    }

    template <> void setField<coral::TimeStamp,40>( const char* buffer, coral::Attribute& attr, unsigned long )
    {
      std::string tsbuf = buffer;
      coral::TimeStamp value( coral::TimeStamp::now() );

      // Check the timestamp size as it affects the format, not the storage
      // 13 or 14 - YYYYMMDDHHMMSS
      // 11 or 12 -   YYMMDDHHMMSS
      //  9 or 10 -   YYMMDDHHMM
      //  7 or  8 - YYYYMMDD
      //  5 or  6 -   YYMMDD
      //  3 or  4 -   YYMM
      //  1 or  2 -   YY

      size_t tsLen = tsbuf.size();

      switch( tsLen )
      {
      case  1:
      case  2: { int year = atoi( tsbuf.c_str() ); value = coral::TimeStamp( 2000+year, 1, 0, 0, 0, 0, 0 ); };
        break;
      case  3:
      case  4: { int year = atoi( tsbuf.substr( 0, 2 ).c_str() ); int month = atoi( tsbuf.substr( 2 ).c_str() ); value = coral::TimeStamp( 2000+year, month, 0, 0, 0, 0, 0 ); };
        break;
      case  5:
      case  6: { int year  = atoi( tsbuf.substr( 0, 2 ).c_str() ); int month = atoi( tsbuf.substr( 2, 2 ).c_str() ); int day   = atoi( tsbuf.substr( 4    ).c_str() ); value = coral::TimeStamp( 2000+year, month, day, 0, 0, 0, 0 ); }; break;
      case  7:
      case  8: { int year  = atoi( tsbuf.substr( 0, 4 ).c_str() ); int month = atoi( tsbuf.substr( 4, 2 ).c_str() ); int day   = atoi( tsbuf.substr( 6    ).c_str() ); value = coral::TimeStamp( year, month, day, 0, 0, 0, 0 ); }; break;
      case  9:
      case 10: { int year  = atoi( tsbuf.substr( 0, 2 ).c_str() ); int month = atoi( tsbuf.substr( 2, 2 ).c_str() ); int day   = atoi( tsbuf.substr( 4, 2 ).c_str() ); int hour  = atoi( tsbuf.substr( 6, 2 ).c_str() ); int mins  = atoi( tsbuf.substr( 8    ).c_str() );
          value = coral::TimeStamp( 2000+year, month, day, hour, mins, 0, 0 ); }; break;
      case 11:
      case 12: {
        int year  = atoi( tsbuf.substr( 0, 2 ).c_str() ); int month = atoi( tsbuf.substr( 2, 2 ).c_str() ); int day   = atoi( tsbuf.substr( 4, 2 ).c_str() );
        int hour  = atoi( tsbuf.substr( 6, 2 ).c_str() ); int mins  = atoi( tsbuf.substr( 8, 2 ).c_str() ); int secs  = atoi( tsbuf.substr( 10   ).c_str() );
        value = coral::TimeStamp( 2000+year, month, day, hour, mins, secs, 0 );
      }; break;
      case 13:
      case 14:
      default: {
        int year  = atoi( tsbuf.substr(  0, 4 ).c_str() ); int month = atoi( tsbuf.substr(  4, 2 ).c_str() ); int day   = atoi( tsbuf.substr(  6, 2 ).c_str() );
        int hour  = atoi( tsbuf.substr(  8, 2 ).c_str() ); int mins  = atoi( tsbuf.substr( 10, 2 ).c_str() ); int secs  = atoi( tsbuf.substr( 12    ).c_str() );
        //           std::cout << "YY: " << year << " MM: " << month << " DD: " << day << " HH: " << hour << " MM: " << mins << " SS: " << secs << std::endl;
        value = coral::TimeStamp( year, month, day, hour, mins, secs, 0 );
      }; break;
      };

      attr.setValueFromAddress( (void*)(&value) );
      attr.setNull( false );
    }

    template <> void setField<coral::Blob,40>( const char* buffer, coral::Attribute& attr, unsigned long size )
    {
      // AV 16.04.2008 Complete fix for bug #34512: move setNull BEFORE data()
      attr.setNull( false );
      coral::Blob&   blobVal = attr.data<coral::Blob>();
      unsigned char* blobRow = (unsigned char*)buffer;
      blobVal.resize( size );
      unsigned char* bvp     = (unsigned char*)blobVal.startingAddress();

      for( long i = 0; i < blobVal.size(); ++i, ++bvp )
        *bvp = blobRow[i];

    }

  }
}

#endif // CORAL_MYSQLACCESS_SETFIELD_H
