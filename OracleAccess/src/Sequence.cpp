#ifdef WIN32
#include <WTypes.h> // fix bug #35683, bug #73144, bug #76882, bug #79849
#endif

#include <memory>
#include "oci.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Exception.h"
#include "CoralBase/MessageStream.h"
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/ICursor.h"
#include "Cursor.h"
#include "OracleErrorHandler.h"
#include "OracleStatement.h"
#include "Sequence.h"
#include "SessionProperties.h"

#ifdef CORAL240SQ

using namespace coral::OracleAccess;

//-----------------------------------------------------------------------------

Sequence::Sequence( boost::shared_ptr<const SessionProperties> sessionProperties,
                    const std::string& schemaName,
                    const coral::ISequenceDescription& description )
  : m_value( description.startValue() )
  , m_sessionProperties( sessionProperties )
  , m_schemaName( schemaName )
  , m_description( description )
{
}

//-----------------------------------------------------------------------------

Sequence::~Sequence()
{
}

//-----------------------------------------------------------------------------

unsigned long long
Sequence::nextValue()
{
  std::auto_ptr<OracleStatement> statement( new OracleStatement( m_sessionProperties, m_schemaName, "SELECT " + m_description.name() + ".nextval AS new_id FROM dual" ) );
  /* empty bind data here */
  coral::AttributeList binddata;
  if( statement->execute( binddata ) )
  {
    coral::AttributeList res;

    res.extend("new_id", typeid(unsigned long long));

    statement->defineOutput( res );
    Cursor* cursor = new Cursor( statement, res );

    if(cursor->next())
    {
      const coral::AttributeList& row = cursor->currentRow();

      m_value = row[0].data<unsigned long long>();

      coral::MessageStream log( "OracleAccess::Sequence" );
      log << coral::Verbose << "Next Value from Sequence : " << m_value << coral::MessageStream::endmsg;
    }

    delete cursor;
  }

  return m_value;
}

//-----------------------------------------------------------------------------

void
Sequence::scan( boost::shared_ptr<const SessionProperties> sessionProperties,
                const std::string& schemaName,
                std::map< std::string, Sequence* >& sequences )
{
  // Execute a SQL statement to retrieve all registered sequences
  /*
    SELECT sequence_name, last_number FROM all_sequences;
  */
  std::auto_ptr<OracleStatement> statement( new OracleStatement( sessionProperties, schemaName, "SELECT sequence_name, last_number FROM all_sequences" ) );
  /* empty bind data here */
  coral::AttributeList binddata;
  if( statement->execute( binddata ) )
  {
    coral::AttributeList res;

    res.extend("SEQUENCE_NAME", typeid(std::string));
    res.extend("ORDER_FLAG", typeid(bool));

    statement->defineOutput( res );

    Cursor* cursor = new Cursor( statement, res );

    while(cursor->next())
    {
      const coral::AttributeList& row = cursor->currentRow();

      const std::string& name = row[0].data<std::string>();
      // Check if the sequence already in the map
      std::map< std::string, Sequence* >::iterator i = sequences.find( name );
      if( i == sequences.end() )
      {
        // A new sequence description
        SequenceDescription desc(name);
        desc.setOrdered( row[1].data<bool>() ? coral::Sequence::Order : coral::Sequence::NoOrder );
        // Add a new sequence to the cache using the description
        Sequence* sequence = new Sequence( sessionProperties, schemaName, desc );

        sequences.insert( std::pair<std::string, Sequence*>( name, sequence));
      }
      coral::MessageStream log( "OracleAccess::Sequence" );
      log << coral::Verbose << "Found SEQUENCE : " << name << coral::MessageStream::endmsg;
    }

    delete cursor;
  }
}

//-----------------------------------------------------------------------------

Sequence*
Sequence::checkIfExists( boost::shared_ptr<const SessionProperties> sessionProperties,
                         const std::string& schemaName,
                         const std::string& name )
{
  Sequence* seq = 0;
  // first create the sql statement
  std::stringstream query;
  query << "SELECT ORDER_FLAG FROM all_sequences WHERE SEQUENCE_NAME = '" << name << "'";

  std::auto_ptr<OracleStatement> statement( new OracleStatement( sessionProperties, schemaName, query.str() ) );
  /* empty bind data here */
  coral::AttributeList binddata;
  if( statement->execute( binddata ) )
  {
    coral::AttributeList res;
    res.extend("ORDER_FLAG", typeid(char));

    statement->defineOutput( res );

    if(statement->fetchNext())
    {
      if(!statement->fetchNext())
      {
        // A new sequence description
        SequenceDescription desc(name);
        desc.setOrdered( res[0].data<char>() == 'Y' ? coral::Sequence::Order : coral::Sequence::NoOrder );
        // Add a new sequnce to the cache using the description
        seq = new Sequence( sessionProperties, schemaName, desc );
      }
      else
      {
        throw coral::Exception( "OracleAccess", "More than one Sequence found", "Sequence::checkIfExists" );
      }
    }
  }
  return seq;
}

//-----------------------------------------------------------------------------

bool
Sequence::checkIfStillExists( boost::shared_ptr<const SessionProperties> sessionProperties,
                              const std::string& schemaName,
                              const std::string& name )
{
  bool ret = false;

  std::stringstream query;
  query << "SELECT ORDER_FLAG FROM all_sequences WHERE SEQUENCE_NAME = '" << name << "'";

  std::auto_ptr<OracleStatement> statement( new OracleStatement( sessionProperties, schemaName, query.str() ) );
  /* empty bind data here */
  coral::AttributeList binddata;
  if( statement->execute( binddata ) )
  {
    coral::AttributeList res;
    res.extend("ORDER_FLAG", typeid(char));

    statement->defineOutput( res );

    if(statement->fetchNext())
    {
      if(!statement->fetchNext())
      {
        ret = true;
      }
      else
      {
        throw coral::Exception( "OracleAccess", "More than one Sequence found", "Sequence::checkIfStillExists" );
      }
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------

#else

#ifdef WIN32
// Fix bug #77061 (LNK4221 warning in Sequence.cpp on Windows - no symbols)
// See http://stackoverflow.com/questions/1822887
namespace { char dummy_Sequence_lnk4221; }
#endif

#endif
