// $Id: CoralServerProxyException.cpp,v 1.1 2009/02/11 18:19:23 avalassi Exp $

// Include files
#include "CoralServerBase/CoralServerProxyException.h"

// Namespace
using namespace coral;

//-----------------------------------------------------------------------------

const std::string CoralServerProxyException::asCALPayload() const
{
  static const unsigned char exceptionCode[2] = { 0x01, 0x00 };
  return std::string( (const char*)exceptionCode, 2 ) + this->what();
}

//-----------------------------------------------------------------------------
