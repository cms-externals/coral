#include "../../src/MySQL_headers.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main( int, char** )
{
  std::cout << "Client version: " << mysql_get_client_info() << std::endl;

  MYSQL c;

  mysql_init( &c );

  mysql_options(&c,MYSQL_READ_DEFAULT_GROUP,"client");

  if( c.options.charset_dir != 0 )
    std::cout << "Charset directory is set to: " << c.options.charset_dir << std::endl;
  else
    std::cout << "No charset directory is set" << std::endl;

  /*
    MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag)
  */
  mysql_real_connect( &c, "pcitdb59.cern.ch", "rado_pool", "rado_pool", "test", 0, 0, 0 );

  MYSQL_RES* result = 0;

  result = mysql_list_fields( &c, "T", 0 );

  if( ! result )
  {
    std::cerr << "Error: " << mysql_error( &c ) << std::endl;
  }

  result = mysql_list_fields( &c, "T", 0 );

  unsigned int num_fields = mysql_num_fields(result);
  MYSQL_FIELD * fields     = mysql_fetch_fields(result);

  for( unsigned int i = 0; i < num_fields; i++)
  {
    MYSQL_FIELD f = fields[i];

    std::cout << i << " " << f.name
      //              << ":default:>>" << f.def
              << ":length:" << f.length
              << std::endl
              << ":max_length:" << f.max_length
              << std::endl
              << ":decimals:" << (f.decimals)
              << std::endl
              << ":numeric:" << IS_NUM( f.type )
              << std::endl
              << ":not null:" << IS_NOT_NULL(f.flags)
              << ":PK:" << IS_PRI_KEY(f.flags)
              << ":UK:" << (f.flags & UNIQUE_KEY_FLAG)
              << ":MULTIKEY:" << (f.flags & MULTIPLE_KEY_FLAG)
              << ":UNSIGNED:" << (f.flags & UNSIGNED_FLAG)
              << ":ZEROFILL:" << (f.flags & ZEROFILL_FLAG)
              << ":BINARY:" << (f.flags & BINARY_FLAG)
              << ":AUTOINCREMENT:" << (f.flags & AUTO_INCREMENT_FLAG)
              << "------------------------------------------------------------------------------"
              << std::endl;
  }

  mysql_free_result( result );

  /*show index from ctmpk;
    +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
    | Table | Non_unique | Key_name | Seq_in_index | Column_name | Collation | Cardinality | Sub_part | Packed | Null | Index_type | Comment |
    +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
    | ctmpk |          0 | PRIMARY  |            1 | id          | A         |           3 |     NULL | NULL   |      | BTREE      |         |
    | ctmpk |          0 | PRIMARY  |            2 | uid         | A         |           3 |     NULL | NULL   |      | BTREE      |         |
    | ctmpk |          0 | kokotar  |            1 | uuid        | A         |           3 |     NULL | NULL   |      | BTREE      |         |
    | ctmpk |          0 | kokotar  |            2 | wid         | A         |           3 |     NULL | NULL   |      | BTREE      |         |
    +-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+
  */
  std::string sql = "SHOW INDEX FROM \"test\".\"T\"";

  result = 0;

  mysql_query( &c, sql.c_str());
  result = mysql_store_result( &c );

  //bool                     lastUnique = false;
  std::string lastIndex;
  std::vector<std::string> columns;
  unsigned seq;

  MYSQL_ROW row;
  while( (row = mysql_fetch_row(result)) )
  {
    std::istringstream is(row[3]); is >> seq;

    if(seq == 1 ) // First column in the index definition
    {
      if( lastIndex.empty() )
      {
        lastIndex = row[2];
        std::cout << "Scanning index " << lastIndex << std::endl;
      }
      else // There is already an index scanned so register it into description
      {
        std::cout << "Creating index " << lastIndex << std::endl;
        //        m_description.createIndex( lastIndex, columns, !lastUnique );
        lastIndex = row[2];
        columns.clear();
        std::cout << "Scanning index " << lastIndex << std::endl;
      }
    }
    std::cout << "Adding column " << row[4] << " into index " << lastIndex << std::endl;

    columns.push_back( row[4] ); // Collect column name
    ///? AK don't care what should happen here
    ///original: lastUnique = *(row[1]);
    //fixed to resolv compiler warning on windows
    //lastUnique = strcmp(row[1], "0") != 0;
  }

  if( !columns.empty() )
    std::cout << "Creating index " << lastIndex << std::endl;
  //    m_description.createIndex( lastIndex, columns, !row[1] );

  mysql_free_result( result );


  mysql_close( &c );

  return 0;
}
