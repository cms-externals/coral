#ifndef RELATIONALACCESS_SEQUENCEDESCRIPTION_H
#define RELATIONALACCESS_SEQUENCEDESCRIPTION_H

#include <string>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx
#include "RelationalAccess/../src/ISequenceDescription.h"

namespace coral
{

#ifdef CORAL240SQ
  /**
   * Class SequenceDescription
   * Transient implementation of the ITableDescription which can be passed
   * as an argument to the createSequence method of the ISchema class.
   */
  class SequenceDescription : public coral::ISequenceDescription
  {

  public:

    /**
     * Default constructor is setting default values
     */
    SequenceDescription( const std::string& name )
      : m_name( name )
      , m_ordertype( Sequence::DefaultOrder )
      , m_startValue( 1 )
    {
    }

    /**
     * Copy constructor
     */
    SequenceDescription( const ISequenceDescription& description )
      : m_name( description.name() )
      , m_ordertype( description.isOrdered() )
      , m_startValue( description.startValue() )
    {
    }

    /**
     * Virtual Destructor
     */
    virtual ~SequenceDescription()
    {
    }

    /**
     * Returns the name of the sequence
     */
    const std::string& name() const
    {
      return m_name;
    }

    /**
     * Returns the order flag for the sequence.
     */
    Sequence::OrderType isOrdered() const
    {
      return m_ordertype;
    }

    /**
     * Returns the start value of the sequence.
     */
    unsigned long startValue() const
    {
      return m_startValue;
    }

    /**
     * Sets the order flag for the sequence.
     */
    void setOrdered( Sequence::OrderType type )
    {
      m_ordertype = type;
    }

    /**
     * Sets the start value of the sequence.
     */
    void setStartValue( unsigned long startvalue )
    {
      m_startValue = startvalue;
    }

  private:

    std::string m_name;

    Sequence::OrderType m_ordertype;

    unsigned long m_startValue;

  };
#endif

}
#endif
