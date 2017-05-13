// -*- C++ -*-
// $Id: NamedInputParametersParser.h,v 1.2 2011/03/22 10:36:50 avalassi Exp $
#ifndef CORAL_FRONTIERACCESS_NAMEDINPUTPARAMTER_PARSER_H
#define CORAL_FRONTIERACCESS_NAMEDINPUTPARAMTER_PARSER_H 1

#include <string>
#include <vector>
#include <map>

namespace coral
{
  namespace FrontierAccess
  {

    typedef std::vector< std::pair<std::string::size_type, std::string::size_type > >                 StrRangePairs;
    typedef std::multimap< std::string, std::pair<std::string::size_type, std::string::size_type > > BVarsPositions;

    class NamedInputParametersParser
    {
    public:
      /** Parse the input string and replace each instance of the
       *  input parameter binding in form of :VAR or :1 by FRONTIER style
       *  using ? marks. e.g. positonal style of variable binding
       *
       *  @param ins The input string to be modified
       *
       *  Returns the modified string
       */
      std::string process( const std::string& ins );

      /** Perform actual search & replace job
       *  @param buffer The copy of the input string edited in place
       *
       * Returns registry of bound variables, their positions & lengths in SQL string
       */
      const BVarsPositions& searchAndMark( std::string& buffer );

      /** Inject the stringified value into SQL as specified by input arguments
       *  @param varName  The bound variable name
       *  @param strValue The actual value converted into a string
       *  @param origSQL  The SQL string to be modified
       *
       *  Returns true if replacement done false otherwise
       */
      bool bindVariable( const std::string& varName, const std::string& strValue, std::string& origSQL );

    protected:
      /** Scans the input string and records begin and end marksers for each section which
       *  is not part of quoted string values like for example 'alksjdfhla \'sdf\'dsfds'
       *  @param input   The input string to be scanned
       *  @param renages The vectors of all recorder marker pairs
       */
      void findNonQuotedRanges( const std::string& input, StrRangePairs& ranges );

    private:
      /// Maping from variable names to their position & length inside SQL
      BVarsPositions m_bvars;
    };
  }
}

#endif // CORAL_FRONTIERACCESS_NAMEDINPUTPARAMTER_PARSER_H
