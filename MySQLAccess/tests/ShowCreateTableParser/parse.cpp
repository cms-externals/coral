#include <algorithm>
#include <iostream>
#include <sstream>
#include "../../src/TableSchemaLoader.h"

const std::string test1buf =
  "CREATE TABLE `MySimpleTable2` (                                                        \
 `fid` mediumint(9) NOT NULL default '0',                                               \
 `tx` float NOT NULL default '0',                                                       \
 `ty` double NOT NULL default '0',                                                      \
 `extra` smallint(6) NOT NULL default '0',                                              \
  UNIQUE KEY `MySimpleTable2_FK_IDX` (`fid`),                                           \
  UNIQUE KEY `extra` (`extra`),                                                         \
  KEY `MySimpleTable2_IDX` (`tx`,`ty`),                                                 \
  CONSTRAINT `MySimpleTable2_FK` FOREIGN KEY (`fid`) REFERENCES `MySimpleTable1` (`id`) \
) TYPE=InnoDB";

const std::string test2buf =
  "CREATE TABLE `MySimpleTable1` (         \
 `id` mediumint(9) NOT NULL default '0', \
 `x` float NOT NULL default '0',         \
 `y` double NOT NULL default '0',        \
 `yy` double default NULL,               \
 `z` double NOT NULL default '0',        \
 `zz` float default NULL,                \
 UNIQUE KEY `z` (`z`)                    \
) TYPE=InnoDB";

const std::string test3buf =
  "CREATE TABLE `T_1` (                                                                                    \
`ID` int(11) NOT NULL default '0',                                                                       \
`x` float NOT NULL default '0',                                                                          \
`Y` double default NULL,                                                                                 \
`Z` double default NULL,                                                                                 \
`b` tinyint(1) default NULL,                                                                             \
`d` date default NULL,                                                                                   \
`data1` varchar(123) default NULL,                                                                       \
`data2` text,                                                                                            \
`ID_FK` int(11) default NULL,                                                                            \
`cc_FK` varchar(255) default NULL,                                                                       \
PRIMARY KEY  (`ID`),                                                                                     \
UNIQUE KEY `Y` (`Y`),                                                                                    \
UNIQUE KEY `C_T1` (`x`,`Z`),                                                                             \
KEY `T_1_FK` (`ID_FK`),                                                                                  \
KEY `T_1_FK_composite` (`ID_FK`,`cc_FK`),                                                                \
CONSTRAINT `T_1_FK` FOREIGN KEY (`ID_FK`) REFERENCES `T_0` (`ID`),                                       \
CONSTRAINT `T_1_FK_composite` FOREIGN KEY `T_1_FK_composite_IDX` (`ID_FK`, `cc_FK`) REFERENCES `T_0` (`ID`, `cc`) \
) TYPE=InnoDB CHARSET=latin1";

const std::string outbuf =
  "Create Table: CREATE TABLE `ibtest11c` (                                 \
  `A` int(11) NOT NULL auto_increment,                                    \
  `D` int(11) NOT NULL default '0',                                       \
  `B` varchar(200) NOT NULL default '',                                   \
  `C` varchar(175) default NULL,                                          \
  PRIMARY KEY  (`A`,`D`,`B`),                                             \
  KEY `B` (`B`,`C`),                                                      \
  KEY `C` (`C`),                                                                                                      \
  CONSTRAINT `0_38775` FOREIGN KEY (`A`, `D`) REFERENCES `ibtest11a` (`A`, `D`) ON DELETE CASCADE ON UPDATE CASCADE,  \
  CONSTRAINT `0_38776` FOREIGN KEY (`B`, `C`) REFERENCES `ibtest11a` (`B`, `C`) ON DELETE CASCADE ON UPDATE CASCADE   \
) TYPE=InnoDB CHARSET=latin1";

/*
  struct ForeignKeyConstraint
  {
  std::string              FKCname;
  std::vector<std::string> FKcolumns;
  std::string              PKtable;
  std::vector<std::string> PKcolumns;
  };

  typedef std::vector<ForeignKeyConstraint> FKConstraints;
*/

namespace coral
{
  namespace MySQLAccess
  {
    bool isEmpty( const std::string& current )
    {
      return current.empty();
    }

    coral::MySQLAccess::FKConstraints parseFKConstraints( const std::string& tableDDL, const char qc='`' )
    {
      typedef std::vector<std::string> FKeys; // List of foreign keys names
      typedef std::vector<std::string> RKeys; // List of referenced keys names

      FKConstraints c; // The list of foreign key constraints to be filled by parsing SHOW CREATE TABLE... output

      if( tableDDL.empty() )
        return c;

      typedef std::vector<std::string> TokenVec;

      TokenVec tokens;
      std::string token;
      // See http://stackoverflow.com/questions/236129/how-to-split-a-string
      std::istringstream iss( tableDDL, std::istringstream::in );
      while( iss >> token ) tokens.push_back( token );

      for( TokenVec::iterator tvit = tokens.begin(); tvit != tokens.end(); ++tvit )
      {
        size_t startpos = tvit->find_first_not_of(" ");
        size_t endpos = tvit->find_last_not_of(" ");
        if ( ( std::string::npos == startpos ) ||
             ( std::string::npos == endpos ) )
          *tvit = "";
        else
          *tvit = tvit->substr( startpos, endpos-startpos+1 );
      }

      tokens.erase( std::remove_if( tokens.begin(), tokens.end(), isEmpty ), tokens.end() );

      size_t slSize    = tokens.size(); // Total number of tokens

      size_t i         = 0;
      size_t currFkc   = 0;
      bool fkcFound  = false;
      bool collectFk = false; // Flag indicating that foreign keys are to be collected

      ForeignKeyConstraint curr;

      std::cout << " The original SQL is:" << std::endl << tableDDL << std::endl;

      while( i < slSize )
      {
        std::cout << "Now parsing ^^^" << tokens[i] << "^^^" << std::endl;

        // Looping over tokens look for CONSTRAINT keyword first
        if( tokens[i] == "CONSTRAINT" )
        {
          // Check if FK constraint, e.g. skip constraint name
          i++;
          if( tokens[i] == "FOREIGN" )
          {
            // No FKC name is given
            fkcFound = true;
            currFkc++;
            i+=2; // Skip KEY keyword
          }
          else if ( tokens[i+1] == "FOREIGN" )
          {
            fkcFound = true;
            curr.FKCname = tokens[i];
            c.push_back( curr );
            currFkc++;
            i+=3; // Skip KEY keyword
          }
          else
          {
            ; // Not a foreign key constraint
          }

        }

        std::string fkcolsbuf; // The string buffer holding joined tokens of FK columns names

        if( fkcFound )
        {
          std::cout << "In FKC parsing ^^^" << tokens[i] << "^^^" << std::endl;

          // Continue parsing by checking if index name is defined or directly the list of foreign key columns
          if( (tokens[i])[0] == qc ) // Paranoia check for identifier delimiter, '`' for version < 4.0.x or '"' for version > 4.1.x
          {
            std::cout << "In FKC parsing found index name " << tokens[i] << std::endl;
            i++;
          }

          if( (tokens[i])[0] == '(' )
          {
            // Now comes the list of FK column names, so we need to eventually join tokens split on ' '
            fkcolsbuf = tokens[i];
            collectFk = true;
          }
          else
          {
            if( collectFk )
            {
              fkcolsbuf += tokens[i];
              if( tokens[i].find(")") != std::string::npos )
                collectFk = false;
            }
          }

          std::cout << "In FKC parsing the list of columns: " << fkcolsbuf << std::endl;
        }

        i++;
      }

      return c;
    }
  }
}

void testFKCparsing( const std::string& buffer )
{
  coral::MySQLAccess::FKConstraints fkcs( coral::MySQLAccess::parseFKConstraints( buffer ) );

  std::cout << "Found " << fkcs.size() << " foreign key constraints" << std::endl;

  for( size_t fkcsi = 0; fkcsi < fkcs.size(); fkcsi++ )
  {
    std::cout << "Foreign key constraint " << fkcs[fkcsi].FKCname << " foreign key ( ";

    for( size_t fki = 0; fki < fkcs[fkcsi].FKcolumns.size(); fki++ )
    {
      std::cout << fkcs[fkcsi].FKcolumns[fki];

      if( fki < fkcs[fkcsi].FKcolumns.size()-1 )
        std::cout << ", ";
    }

    std::cout << " ) references table " << fkcs[fkcsi].PKtable << " with primary key ( ";

    for( size_t pki = 0; pki < fkcs[fkcsi].PKcolumns.size(); pki++ )
    {
      std::cout << fkcs[fkcsi].PKcolumns[pki];

      if( pki < fkcs[fkcsi].PKcolumns.size()-1 )
        std::cout << ", ";
    }

    std::cout << " )" << std::endl;
  }
}

int main( int, char** )
{

  testFKCparsing( test1buf );
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  testFKCparsing( test2buf );
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  testFKCparsing( test3buf );
  std::cout << "-------------------------------------------------------------------------------" << std::endl;
  testFKCparsing( outbuf );

  return 0;
}
