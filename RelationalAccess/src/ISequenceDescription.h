#ifndef RELATIONALACCESS_ISEQUENCEDESCRIPTION_H
#define RELATIONALACCESS_ISEQUENCEDESCRIPTION_H

#include <string>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

namespace coral
{

#ifdef CORAL240SQ

  namespace Sequence
  {
    enum OrderType { DefaultOrder, Order, NoOrder };
  }

  /**
   * Class ISequenceDescription
   * Abstract interface for the description of an SQL sequence.
   */
  class ISequenceDescription
  {

  public:

    /**
     * Returns the name of the sequence.
     */
    virtual const std::string& name() const = 0;

    /**
     * Returns the order flag for the sequence.
     */
    virtual coral::Sequence::OrderType isOrdered() const = 0;

    /**
     * Return the start value of the sequence.
     */
    virtual unsigned long startValue() const = 0;

  };
#endif

}
#endif
