#ifndef CORALCOMMON_COMMANDLINE_H
#define CORALCOMMON_COMMANDLINE_H

#include <string>
#include <vector>
#include <map>
#include "CoralBase/Exception.h"

namespace coral {


  // structure defining the supported option parameters
  struct Option {

    typedef enum { BOOLEAN,
                   STRING } OptionType;
    // default constructor
    Option();

    virtual ~Option(){}

    // constructor
    explicit Option(const std::string& name);

    //copy constructor
    Option(const Option& anOpt);

    //assignment operator
    Option& operator=(const Option& anOpt);

    // type of the option (simple boolean or parameter with an associated value)
    OptionType type;

    // label defining the option on the command line
    std::string flag;

    // name associated to the option
    std::string name;

    // explanation sentence appearing on the help
    std::string helpEntry;

    // default value associated to the property when the option is not set
    std::string defaultValue;

    // flag to declare the option as required
    bool required;

    // flag to declare the option as exclusive
    bool exclusive;

  };

  // structure defining the supported option parameters
  class Command : public Option {

  public:
    // default constructor
    Command();

    // constructor
    explicit Command(const std::string& name);

    virtual ~Command(){}

    //copy constructor
    Command(const Command& anOpt);

    //assignment operator
    Command& operator=(const Command& anOpt);

    // add a dependent option (defined in an Option instance)
    void addOption(const std::string& optionName, bool required=false);

    // read-only access to the dependent option list
    const std::map<std::string,bool>& dependentOptions() const;

  private:

    std::map<std::string,bool> m_dependentOptions;

  };

  // base class for the option failures
  class OptionException  : public coral::Exception {
  public:
    /// Constructor
    explicit OptionException( const std::string& message );
    /// Constructor
    OptionException( const std::string& message, const std::string& context );
  };

  // exception class for the failures in parsing procedure
  class OptionParseException  : public OptionException {
  public:
    /// Constructor
    explicit OptionParseException( const std::string& message );
  };

  // exception class for the invalid option
  class InvalidOptionException  : public OptionException {
  public:
    /// Constructor
    explicit InvalidOptionException( const std::string& optionName );
  };

  // exception class for the invalid option
  class MissingRequiredOptionException  : public OptionException {
  public:
    /// Constructor
    explicit MissingRequiredOptionException( const std::string& optionName );
    /// Constructor
    MissingRequiredOptionException();
  };

  // exception class
  class ExclusiveOptionException  : public OptionException {
  public:
    /// Constructor
    ExclusiveOptionException( const std::string& option1,
                              const std::string& option2 );
  };

  /**
     Utility class for handling input parameters of command line executables.
     It provides parsing of the command line parameters according to pre-defined options, automated creatiion of help, error generation on misusage.

  */
  class CommandLine {

  public:

    // marker for the option strings
    static const char& optionMarker();

    static const Option& helpOption();

  public:

    // constructor - requires an array of Option structures
    explicit CommandLine(const std::vector<Option>& supportedOptions);

    // constructor - requires an array of Option structures
    CommandLine(const std::vector<Option>& supportedOptions, const std::vector<Command>& commands);

    virtual ~CommandLine();

    // activate the parsing of the command line parameters
    bool parse(int argc,char** argv);



    // returns a map of the option set by the user with their associated values
    const std::map<std::string,std::string>& userOptions() const;

    // returns true if the option has been selected
    bool find( const std::string& optionName) const;

    // returns the value of the option,if selected.
    const std::string& optionValue( const std::string& optionName ) const;

    // returns the executable name of the command
    const std::string& commandName() const;

    // returns the command name selected by the user, in command mode
    const std::string& userCommand() const;

    // displays the available options defined
    void help(std::ostream& out) const;

    // displays the available options defined
    void help(const std::string& command, std::ostream& out) const;

    // displays the options set by the user with their associated values
    void dumpUserOptions(std::ostream& out) const;

  private:

    static bool parse(int argc, char** argv, std::map<std::string,std::string>& inputOptions);

    bool validate(const std::map<std::string,std::string>& userOptions,
                  const std::map<std::string,Option*>& supportedOptions);

    void printOption(const Option& opt, std::ostream& out) const;

    void printCommand(const Command& cmd, std::ostream& out) const;

  private:

    // array of option objects re-copied
    std::vector<Option*> m_options;

    // array of supported option (described by Option structures)
    std::map<std::string,Option*> m_supportedOptions;

    // map op option set by the user with their associated values
    std::map<std::string,std::string> m_userOptions;

    // executable name of the command
    std::string m_commandName;

    // executable name of the command
    std::string m_userCommand;

    // array of command objects
    std::vector<Command*> m_commands;

    // array of supported commands (described by Command structures)
    std::map<std::string,Command*> m_supportedCommands;

    mutable std::pair<std::string,const std::string* > m_currentOption;

  };


}

#endif
