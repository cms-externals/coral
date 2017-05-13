// $Id: NamedInputParametersParser.cpp,v 1.11 2011/03/22 10:29:54 avalassi Exp $
#include "MySQL_headers.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "NamedInputParametersParser.h"

namespace coral
{
  namespace MySQLAccess
  {
    std::string NamedInputParametersParser::process( const std::string& inputString )
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
      std::string::size_type equote   = 0;
      std::string::size_type nbquote  = 0;
      std::string::size_type nequote  = 0;

      for( std::string::size_type i = 0; i < len; i++ )
      {
        if( inQuotes )
        {
          if( input[i] == '\'')
          {
            if( input[i-1] != '\\' || ( input[i-1] == '\\' && input[i-2] == '\\' ) )
            {
              inQuotes = false;
              equote   = i;
              nbquote  = i+1;
              nequote  = nbquote;
            }
          }

          equote++;
        }
        else
        {
          if( input[i] == '\'' )
          {
            if( input[i-1] != '\\' || ( input[i-1] == '\\' && input[i-2] == '\\' ) )
            {
              inQuotes = true;
              bquote   = i;
              ranges.push_back( std::pair < std::string::size_type, std::string::size_type > ( nbquote, nequote-nbquote-1 ) );
              nequote  = i;
              continue;
            }
          }

          ++nequote;
        }
      }

      if( inQuotes )
      {
        std::stringstream os;
        os
          << "Unmatched quotes starting at: " << (unsigned long)bquote <<  std::endl
          << input << std::endl
          << std::string( bquote, ' ' ) << "-" << std::endl
          << std::string( bquote, '_' ) << "|" << std::endl
          << std::endl;
        throw std::logic_error( os.str() );
      }

      ranges.push_back( std::pair < std::string::size_type, std::string::size_type > ( nbquote, nequote-nbquote ) );
    }

    const BVars NamedInputParametersParser::searchAndMark( std::string& toFix )
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

        std::string::size_type sbeg     = 0;
        std::string::size_type send     = 0;
        std::string::size_type slen     = 0;

        std::string::size_type bvarpos  = 0;

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

          BindVariable bv =  { bvarpos, sbeg, slen };
          m_bvars.insert( std::pair< std::string, BindVariable > ( realName, bv ) );
          bvarpos++;

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

    bool NamedInputParametersParser::bindVariable( const std::string& varName, const std::string& strValue, std::string& origSQL )
    {
      bool done = false;

      StrRangePairs notQuoted;

      // Remember SQL string length before inplace replacements
      std::string::size_type origLen     = origSQL.size();

      findNonQuotedRanges( origSQL, notQuoted );

      // Search each fragment for a bound variable
      StrRangePairs::size_type frgmts = notQuoted.size();

      for( StrRangePairs::size_type range = 0; range < frgmts; range++ )
      {
        // The current length after possible in-place replacements
        std::string::size_type currLen   = origSQL.size();
        // Get the length normalization
        std::string::size_type corrector = currLen - origLen;

        std::string::size_type begin     = notQuoted[range].first  + corrector;
        std::string::size_type end       = notQuoted[range].second + corrector;

        if( end > currLen || end == std::string::npos )
          end = currLen;

        // Check if are running out of bounds
        if( begin > currLen )
        {
          std::ostringstream os;
          os << "Ran out of bounds, original SQL " << origLen << " chars currentSQL " << currLen << " asked for substr from " << begin << " to " << end;
          throw std::logic_error( os.str() );
        }

        std::string fragment = origSQL.substr( begin, end );
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
    }

  } // FrontierAccess
} // pool
