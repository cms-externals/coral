// $Id: hexstring.h,v 1.4.2.2 2011/12/10 22:47:56 avalassi Exp $
#ifndef CORALSERVERBASE_HEXSTRING_H
#define CORALSERVERBASE_HEXSTRING_H 1

// Include files
#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <iostream>
#include <string>

/** @file hexstring.h
 *
 *  Simple methods for converting a string into a hex dump.
 *
 *  @author Andrea Valassi
 *  @date   2009-01-21
 */

namespace coral
{

  //-------------------------------------------------------------------------

  inline const std::string hexstring( const char* pData,
                                      size_t nData )
  {
    std::string hexData = "0x";
    for ( size_t p=0; p<nData; p++ )
    {
      char xTmp[2];
      unsigned char cTmp = *(pData+p); // NB Need __unsigned__ char conversion!
      sprintf( xTmp, "%02X", cTmp );
      hexData += xTmp;
    }
    return hexData;
  }

  //-------------------------------------------------------------------------

  inline const std::string hexstring( const unsigned char* pData,
                                      size_t nData )
  {
    return hexstring( (const char*) pData, nData );
  }

  //-------------------------------------------------------------------------

  inline const std::string hexstring( const std::string& data )
  {
    return hexstring( data.c_str(), data.size() );
  }

  //-------------------------------------------------------------------------

}
#endif // CORALSERVERBASE_HEXSTRING_H
