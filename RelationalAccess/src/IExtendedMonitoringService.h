#ifndef RELATIONALACCESS_IEXTENDEDMONITORINGSERVICE_H
#define RELATIONALACCESS_IEXTENDEDMONITORINGSERVICE_H

#include "RelationalAccess/IMonitoringService.h"

namespace coral
{

  // Forward declaration
  class AttributeList;

  namespace monitor
  {

    /** @typedef RecordData
     *
     *  Container class to store events of different types
     *
     *  @author Alexander Kalkhof
     *  @date   2010-09-02
     */
    class RecordData
    {

    public:

      /**
       * Constructor
       * @param type A byte value for the kind of Data type (should be unique)
       */
      RecordData(char type)
        : m_type( type )
      {}

      /**
       * Destructor
       */
      virtual ~RecordData()
      {}

      /**
       * Return the type
       */
      char type() const
      {
        return m_type;
      }

    private:

      char m_type;

    };

    /** @typedef IExtendedMonitoringService
     *
     *  Extended IMonitoringService supporting generic AttributeList events.
     *  The only difference versus IMonitoringService is the record method
     *  with an AttributeList argument: eventually this will be added to
     *  the default IMonitoringService, but this is postponed as it would
     *  require a binary-incompatible public API change versus CORAL_2_3_3.
     *
     *  @author Andrea Valassi, Alexander Kalkhof
     *  @date   2010-09-02
     */
    class IExtendedMonitoringService : virtual public IMonitoringService
    {

    public:

      /**
       * Sets the level
       * @param contextKey The session ID for which to make the report
       * @param level      The monitoring level ( Default, Debug, Trace )
       */
      virtual void setLevel( const std::string& contextKey,
                             monitor::Level level ) = 0;

      /**
       * Return current monitoring level
       */
      virtual monitor::Level level( const std::string& contextKey ) const = 0;

      /**
       * Return monitoring activity status
       */
      virtual bool active( const std::string& contextKey ) const = 0;

      /**
       * Enable monitoring for the given session
       */
      virtual void enable( const std::string& contextKey ) = 0;

      /**
       * Disable monitoring for the given session
       */
      virtual void disable( const std::string& contextKey ) = 0;

      /**
       * Records an event without a payload ( time event for example )
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description ) = 0;

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description,
                           int data ) = 0;

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description,
                           long long data ) = 0;

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description,
                           double data ) = 0;

      /**
       * Records an event with a payload
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description,
                           const std::string& data ) = 0;

      /**
       * Records an event with a generic RecordData payload
       * the pointer will be released by the MonitoringService.
       * == This is the only extension of IMonitoringService. ==
       */
      virtual void record( const std::string& contextKey,
                           monitor::Source source,
                           monitor::Type type,
                           const std::string& description,
                           const RecordData* data ) = 0;

      /**
       * Return the cont reference to a reporting component which allows to
       * query & report the collected monitoring data
       */
      virtual const IMonitoringReporter& reporter() const = 0;

    protected:

      /// Protected empty destructor
      virtual ~IExtendedMonitoringService() {}

    };

  }

}
#endif
