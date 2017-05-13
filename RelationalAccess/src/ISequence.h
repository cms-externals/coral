#ifndef RELATIONALACCESS_ISEQUENCE_H
#define RELATIONALACCESS_ISEQUENCE_H

#include <string>
#include "CoralBase/VersionInfo.h" // Relevant for #ifdef CORAL240xx

namespace coral
{

#ifdef CORAL240SQ

  // Forward declaration
  class ISequenceDescription;

  /**
   * Class ISequence
   *
   * Abstract class for a sequence
   */
  class ISequence
  {

  public:

    /**
     * returns the description
     */
    virtual const ISequenceDescription& description() const = 0;

    /**
     * gets a new sequence number
     */
    virtual unsigned long long nextValue() = 0;

  };
#endif

}
#endif
