// $Id: NamedInputParametersParser.cpp,v 1.5 2011/03/22 10:36:50 avalassi Exp $
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "NamedInputParametersParser.h"

using namespace coral::FrontierAccess;

//-----------------------------------------------------------------------------

std::string
NamedInputParametersParser::process( const std::string& inputString )
{
  std::string processed = inputString;

  searchAndMark( processed );

  return processed;
}

void NamedInputParametersParser::findNonQuotedRanges( const std::string& input, StrRangePairs& ranges )
{
  bool inQuotes = false;
  std::string::size_type len      = input.size();
  std::string::size_type bquote   = 0;
  //std::string::size_type equote   = 0;
  std::string::size_type nbquote  = 0;
  std::string::size_type nequote  = 0;

  for( std::string::size_type i = 0; i < len; i++ )
  {
    if( inQuotes ) {
      if( input[i] == '\'' && input[i-1] != '\\' ) {
        inQuotes = false;
        //equote   = i;
        nbquote  = i+1;
        nequote  = nbquote;
      } else {
        continue;
      }
    } else {
      if( input[i] == '\'' ) {
        inQuotes = true;
        bquote   = i;
        ranges.push_back(
                         std::pair < std::string::size_type, std::string::size_type > (
                                                                                       nbquote,
                                                                                       nequote-nbquote-1
                                                                                       )
                         );
        nequote  = i;
        continue;
      } else {
        ++nequote;
      }
    }
  }

  if( inQuotes ) {
    std::stringstream os;
    os
      << "Unmatched quotes starting at: " << (unsigned long)bquote <<  std::endl
      << input << std::endl
      << std::string( bquote, ' ' ) << "-" << std::endl
      << std::string( bquote, '_' ) << "|" << std::endl
      << std::endl;
    throw std::logic_error( os.str() );
  }

  ranges.push_back(
                   std::pair < std::string::size_type, std::string::size_type > (
                                                                                 nbquote,
                                                                                 nequote-nbquote
                                                                                 )
                   );
}

//-----------------------------------------------------------------------------

const BVarsPositions&
NamedInputParametersParser::searchAndMark( std::string& toFix )
{
  // We do new scan for bound variables
  m_bvars.clear();

  StrRangePairs notQuoted;

  findNonQuotedRanges( toFix, notQuoted );

  //// Perform substitution for each fragment
  StrRangePairs::size_type frgmts = notQuoted.size();

  for( StrRangePairs::size_type range = 0; range < frgmts; range++ )
  {
    std::string::size_type begin = notQuoted[range].first;
    std::string::size_type end   = notQuoted[range].second;

    std::string fragment = toFix.substr( begin, end );
    std::string::size_type len      = fragment.size();

    std::string::size_type sbeg  = 0;
    std::string::size_type send  = 0;
    std::string::size_type slen  = 0;

    std::string realName = "";

    while( ( sbeg = fragment.find( ':', sbeg ) ) != std::string::npos )
    {
      send = sbeg;
      bool sOpen = false;
      while( (send = fragment.find( '"', send ) ) != std::string::npos )
      {
        send++;
        if( ! sOpen )
        {
          sOpen = true;
        }
        else
        {
          sOpen = false;
          break;
        }
      }

      slen = send-sbeg;

      if( send == std::string::npos )
        send = len;

      realName = fragment.substr( sbeg+2,slen-3);

      //           std::cout << "In fragment from " << begin << " to " << end << std::endl
      //                     << "                                                                                                      1         1         1" << std::endl
      //                     << "            1         2         3         4         5         6         7         8         9         0         1         2" << std::endl
      //                     << "  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890" << std::endl
      //                     << ">>" << fragment << "<< of original SQL string" << std::endl
      //                     << ">>" << toFix    << "<<"                        << std::endl
      //                     << " found bound variable "                        << std::endl
      //                     << ">>" << fragment.substr( sbeg,slen ) << "<< at position " << sbeg << " " << slen << " chars long and real name " << realName << std::endl;

      m_bvars.insert( std::pair< std::string, std::pair<std::string::size_type, std::string::size_type > > ( realName, std::pair< std::string::size_type, std::string::size_type > ( sbeg, slen ) ) );

      //           fragment.erase( sbeg, slen );

      //           if( sbeg > fragment.size() )
      //             fragment.insert( sbeg-1, slen, ' ' );
      //           else
      //             fragment.insert( sbeg, slen, ' ' );

      //           fragment.replace( sbeg, slen, "?" );
      sbeg++;
    }

    //         toFix.replace( begin, end, fragment );
  }

  return m_bvars;
}

//-----------------------------------------------------------------------------

bool
NamedInputParametersParser::bindVariable( const std::string& varName, const std::string& strValue, std::string& origSQL )
{
  // Create real name
  std::string name;
  name.append("\"");
  name.append(varName);
  name.append("\"");
  // Result string
  std::string res;
  // Copy the original statement into the stream
  std::stringstream strstr(origSQL);
  // Start parsing
  std::string s01;
  if( getline(strstr, s01, ':') )
  {
    // The first token was found
    // Add the previous part to the result string
    res.append(s01);
    // Continue to find the other tokens
    while( getline(strstr, s01, ':') )
    {
      // Find the name, the name should be the first chars
      if( s01.compare( 0, name.size(), name ) == 0 )
      {
        // Add the value
        res.append( strValue );
        // Add the rest of the s01 string
        res.append( s01.substr(name.size()) );
      }
      else
      {
        // The token was not the name
        // Add the delimiter and the rest of the string
        res.append( ":" );
        res.append( s01 );
      }
    }
    // Override the original string with the result
    origSQL = res;
    // We found at least one bind variable
    return true;
  }
  // No bind variable was found
  return false;
  /*

      bool done = false;

      StrRangePairs notQuoted;

      findNonQuotedRanges( origSQL, notQuoted );

      // Search each fragment for a bound variable
      StrRangePairs::size_type frgmts = notQuoted.size();

      for( StrRangePairs::size_type range = 0; range < frgmts; range++ )
      {
        std::string::size_type begin = notQuoted[range].first;
        std::string::size_type end   = notQuoted[range].second;

        std::string            fragment = origSQL.substr( begin, end );
        std::string::size_type len      = fragment.size();

        std::string::size_type sbeg  = 0;
        std::string::size_type send  = 0;
        std::string::size_type slen  = 0;

        while( ( sbeg = fragment.find( ':', sbeg ) ) != std::string::npos )
        {
          send = sbeg;
          bool sOpen = false;
          while( (send = fragment.find( '"', send ) ) != std::string::npos )
          {
            send++; if( ! sOpen ) { sOpen = true; } else { sOpen = false; break; }
          }

          slen = send-sbeg;

          if( send == std::string::npos ) send = len;

          std::string decorName = ":\"" + varName + "\"";
          if( decorName == fragment.substr( sbeg, slen ) )
          {
            fragment.erase( sbeg, slen );

            if( sbeg > fragment.size() )
              fragment.insert( sbeg-1, slen, ' ' );
            else
              fragment.insert( sbeg, slen, ' ' );

            fragment.replace( sbeg, slen, strValue );

            origSQL.replace( begin, end, fragment );

            done = true;
          }

          sbeg++;
        }
      }

      return done;
  */
}
