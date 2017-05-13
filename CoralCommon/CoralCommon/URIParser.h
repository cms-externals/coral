#ifndef CORALCOMMON_URI_PARSER_H
#define CORALCOMMON_URI_PARSER_H

#include <string>

namespace coral {

  /**
   * @class URIParser URIParser.h CoralCommon/RelationalURIParser.h
   *
   * A URI parser for accessing relational databases.
   * A contact string has the form:
   *
   * technology_protocol://hostName:portNumber/databaseOrSchemaName
   *
   * Where technology can be oracle, mysql, odbc, etc. (all small letters).
   * The protocol is optional and can be file, http, etc.
   * The port number and service name are optional as well.
   *
   */
  class URIParser {
  public:
    /// Constructor
    URIParser();

    /// Destructor
    ~URIParser();

    /// Sets the URI string
    void setURI( const std::string& URIString );

    /// Gets the current URI string
    const std::string& getURI() const;

    /// Returns the technology name
    const std::string& technology() const;

    /// Retrieves the protocol name
    const std::string& protocol() const;

    /// Returns the host name
    const std::string& hostName() const;

    /// Returns the database or schema name
    const std::string databaseOrSchemaName() const;

    /// Returns the network port number
    int portNumber() const;

  private:
    /// The URI string
    std::string m_URI;

    /// The technology name
    std::string m_technology;

    /// The protocol name
    std::string m_protocol;

    /// The host name
    std::string m_hostName;

    /// The database or schema name
    std::string m_databaseOrSchemaName;

    /// The port number
    int m_portNumber;

    /// Flag indicating whether parsing needs to be done on the URI string
    bool m_needsParsing;

  private:
    /// Parses the string
    void parse();
  };

}

#endif
