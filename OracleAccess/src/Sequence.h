#ifndef ORACLEACCESS_SEQUENCE_H
#define ORACLEACCESS_SEQUENCE_H

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/../src/ISequence.h" // temporary
#include "RelationalAccess/../src/SequenceDescription.h"

namespace coral
{

  namespace OracleAccess
  {
    // Forward declaration
    class SessionProperties;

#ifdef CORAL240SQ
    /**
     * Class Sequence
     *
     * OracleAccess implementation of ISequence
     */
    class Sequence : public coral::ISequence
    {
      // Only schema class can create/delete
      friend class Schema;

    public:

      /**
       * returns the description
       */
      const ISequenceDescription& description() const
      {
        return m_description;
      }

      /**
       * gets a new sequence number
       */
      unsigned long long nextValue();

      /**
       * Provide an interface to scan for all registered seqences
       */
      static void scan( boost::shared_ptr<const SessionProperties> sessionProperties,
                        const std::string& schemaName,
                        std::map< std::string, Sequence* >& sequences );

      /**
       * Check if the sequence is still exists
       */
      static Sequence* checkIfExists( boost::shared_ptr<const SessionProperties> sessionProperties,
                                      const std::string& schemaName,
                                      const std::string& name );

      /**
       * Check if the sequence is still exists
       */
      static bool checkIfStillExists( boost::shared_ptr<const SessionProperties> sessionProperties,
                                      const std::string& schemaName,
                                      const std::string& name );

    private:

      /**
       * Private constructor, can only be called by Schema
       */
      Sequence( boost::shared_ptr<const SessionProperties> sessionProperties,
                const std::string& schemaName,
                const ISequenceDescription& description );

      /**
       * Private destructor, can only be called by Schema
       */
      virtual ~Sequence();

    private:

      unsigned long long m_value;

      /// The session properties
      boost::shared_ptr<const SessionProperties> m_sessionProperties;

      /// The schema name for this sequence
      const std::string m_schemaName;

      coral::SequenceDescription m_description;

    };
#endif

  }

}
#endif
