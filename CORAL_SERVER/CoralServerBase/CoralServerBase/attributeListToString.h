#ifndef CORALSERVERBASE_ATTRIBUTELISTTOSTRING_H
#define CORALSERVERBASE_ATTRIBUTELISTTOSTRING_H 1

// Include files
#include <iostream>
#include <sstream>
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

namespace coral
{

  /** stringToAttributeList
   *
   *  @author Andrea Valassi
   *  @date   2007-12-05
   */

  inline const coral::AttributeList stringToAttributeList( const std::string& sAL )
  {
    //std::cout << "Convert to AttributeList: " << sAL << std::endl;
    coral::AttributeList al;
    if ( sAL=="" ) return al;
    // Split the string using the ", " separator
    // The result is a vector of Attribute's as strings
    std::string sAL2 = sAL;
    std::vector<std::string> sAs;
    std::string::size_type sepPosition = sAL2.find( ", " );
    while ( sepPosition != std::string::npos )
    {
      sAs.push_back( sAL2.substr( 0, sepPosition ) );
      //std::cout << "New attribute as string: '" << sAL2.substr( 0, sepPosition ) << "'" << std::endl;
      sAL2 = sAL2.substr( sepPosition + 2 ); // 2 is the separator size
      sepPosition = sAL2.find( ", " );
    }
    //std::cout << "New attribute as string: '" << sAL2.substr( 0, sepPosition ) << "'" << std::endl;
    sAs.push_back( sAL2.substr( 0, sepPosition ) );
    // Iterate over the Attribute's as strings
    for ( std::vector<std::string>::const_iterator sA = sAs.begin(); sA != sAs.end(); sA++ )
    {
      // Strip the leading "[" and trailing "]"
      std::string sA2 = (*sA).substr( 1, (*sA).size()-2 );
      // Split into specification and value using the " : " separator
      sepPosition = sA2.find( " : " );
      std::string sAS = sA2.substr( 0, sepPosition );
      std::string sAV = sA2.substr( sepPosition + 3 ); // 3 is the separator size
      // Split using the last appearance of the " (" separator
      sepPosition = sAS.rfind( " (" );
      std::string aName = sAS.substr( 0, sepPosition );
      std::string aType = sAS.substr( sepPosition + 2 ); // 2 is the separator size
      aType=aType.substr( 0, aType.size()-1 ); // remove trailing ")"
      //std::cout << "Type='" << aType << "', Name='" << aName << "', Value='" << sAV << "'" << std::endl;
      al.extend(aName,aType);
      if ( aType == "int" )
        al[aName].setValue<int>( atoi(sAV.c_str()) );
      else if ( aType == "short" )
        al[aName].setValue<short>( atoi(sAV.c_str()) );
      else if ( aType == "float" )
        al[aName].setValue<float>( (float)atof(sAV.c_str()) );
      else if ( aType == "string" )
        al[aName].setValue<std::string>( sAV.c_str() );
      else if ( aType == "bool" )
        al[aName].setValue<bool>( sAV == "TRUE" );
      else throw Exception( "Type "+aType+" not implemented yet", "stringToAttributeList", "coral::CoralProtocol" );
    }
    return al;
  }

  /** stringsToAttributeLists
   *
   *  @author Andrea Valassi
   *  @date   2007-12-05
   */

  inline const std::vector<coral::AttributeList> stringsToAttributeLists( const std::vector<std::string>& sALs )
  {
    //std::cout << "Convert to AttributeLists" << std::endl;
    std::vector<coral::AttributeList> als;
    for ( std::vector<std::string>::const_iterator sAL = sALs.begin(); sAL != sALs.end(); sAL++ )
    {
      als.push_back( stringToAttributeList(*sAL) );
    }
    return als;
  }

  /** attributeListToString
   *
   *  @author Andrea Valassi
   *  @date   2005-07-20
   */

  inline const std::string attributeListToString( const coral::AttributeList& data )
  {
    std::ostringstream dataStream;
    data.toOutputStream( dataStream );
    return dataStream.str();
  }

  /** attributeListsToStrings
   *
   *  @author Andrea Valassi
   *  @date   2008-04-22
   */

  inline const std::vector<std::string> attributeListsToStrings( const std::vector<coral::AttributeList>& rows )
  {
    std::vector<std::string> rowsAS;
    for ( std::vector<coral::AttributeList>::const_iterator row = rows.begin(); row != rows.end(); row++ )
    {
      rowsAS.push_back( attributeListToString(*row) );
    }
    return rowsAS;
  }

}
#endif // CORALSERVERBASE_ATTRIBUTELISTTOSTRING_H
