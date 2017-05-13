#include "CoralCommon/CommandLine.h"
#include <sstream>
#include <iostream>

coral::Option::Option() :
  type(BOOLEAN),
  flag(""),
  name(""),
  helpEntry(""),
  defaultValue(""),
  required(false),
  exclusive(false){
}

coral::Option::Option(const std::string& aName) :
  type(BOOLEAN),
  flag(""),
  name(aName),
  helpEntry(""),
  defaultValue(""),
  required(false),
  exclusive(false){
}

coral::Option::Option(const Option& anOpt) :
  type(anOpt.type),
  flag(anOpt.flag),
  name(anOpt.name),
  helpEntry(anOpt.helpEntry),
  defaultValue(anOpt.defaultValue),
  required(anOpt.required),
  exclusive(anOpt.exclusive){
}

coral::Option&
coral::Option::operator=(const Option& anOpt){
  type = anOpt.type;
  flag = anOpt.flag;
  name = anOpt.name;
  helpEntry = anOpt.helpEntry;
  defaultValue = anOpt.defaultValue;
  required = anOpt.required;
  exclusive = anOpt.exclusive;
  return *this;
}

// default constructor
coral::Command::Command() : Option(),m_dependentOptions(){
  exclusive = true;
}

// constructor
coral::Command::Command(const std::string& name) : Option(name),m_dependentOptions(){
  exclusive = true;
}

//copy constructor
coral::Command::Command(const Command& anOpt) : Option(anOpt),m_dependentOptions(anOpt.m_dependentOptions){
}

//assignment operator
coral::Command& coral::Command::operator=(const Command& anOpt){
  Option::operator=(anOpt);
  m_dependentOptions = anOpt.m_dependentOptions;
  return *this;
}

// add a dependent option (defined in an Option instance)
void coral::Command::addOption(const std::string& optionName, bool required){
  if(optionName!=name) m_dependentOptions.insert(std::make_pair(optionName,required));
}

// read-only access to the dependent option list
const std::map<std::string,bool>& coral::Command::dependentOptions() const {
  return m_dependentOptions;
}

coral::OptionException::OptionException( const std::string& msg ) :
  coral::Exception( msg, "CommandLine::parse", "CoralCommon" ){
}

coral::OptionException::OptionException( const std::string& msg,
                                         const std::string& context ) :
  coral::Exception( msg, context, "CoralCommon" ){
}

coral::OptionParseException::OptionParseException( const std::string& msg ) :
  coral::OptionException( msg ){
}

coral::InvalidOptionException::InvalidOptionException( const std::string& optionName ) :
  coral::OptionException( "Unsupported option \""+optionName +"\" found."){
}

coral::MissingRequiredOptionException::MissingRequiredOptionException( const std::string& optionName ) :
  coral::OptionException( "Required parameter \""+optionName+"\" is missing."){
}

coral::MissingRequiredOptionException::MissingRequiredOptionException() :
  coral::OptionException( "A Required option choice is missing."){
}

coral::ExclusiveOptionException::ExclusiveOptionException( const std::string& option1,
                                                           const std::string& option2 ) :
  coral::OptionException( "Invalid choice: options \""+option1+"\" and \""+option2+"\" are incompatible."){
}


const char& coral::CommandLine::optionMarker(){
  static char s_optionMarker('-');
  return s_optionMarker;
}

const coral::Option& coral::CommandLine::helpOption(){
  static Option s_helpOpt;
  s_helpOpt.flag = "-help";
  s_helpOpt.type = Option::BOOLEAN;
  s_helpOpt.name = "help";
  s_helpOpt.helpEntry = "display the help";
  return s_helpOpt;
}

coral::CommandLine::CommandLine(const std::vector<Option>& supportedOptions) :
  m_options(),
  m_supportedOptions(),
  m_userOptions(),
  m_commandName(""),
  m_userCommand(""),
  m_commands(),
  m_supportedCommands(),
  m_currentOption("",0){
  std::vector<Option>::const_iterator iOpt;
  bool helpFound = false;
  for(iOpt = supportedOptions.begin(); iOpt!=supportedOptions.end(); iOpt++) {
    Option* opt = new Option(*iOpt);
    m_options.push_back(opt);
    if(iOpt->name == helpOption().name) helpFound = true;
    m_supportedOptions.insert(std::make_pair(iOpt->flag,opt));
    std::string optNameKey("--");
    m_supportedOptions.insert(std::make_pair(optNameKey.append(iOpt->name),opt));
  }
  if(!helpFound) {
    Option* helpOpt(new Option(helpOption()));
    m_options.push_back(helpOpt);
    m_supportedOptions.insert(std::make_pair(helpOption().flag,helpOpt));
    std::string optNameKey("--");
    m_supportedOptions.insert(std::make_pair(optNameKey.append(helpOption().name),helpOpt));
  }
}

coral::CommandLine::CommandLine(const std::vector<Option>& supportedOptions,
                                const std::vector<Command>& commands) :
  m_options(),
  m_supportedOptions(),
  m_userOptions(),
  m_commandName(""),
  m_userCommand(""),
  m_commands(),
  m_supportedCommands(){
  std::vector<Option>::const_iterator iOpt;
  bool helpFound = false;
  for(iOpt = supportedOptions.begin(); iOpt!=supportedOptions.end(); iOpt++) {
    Option* opt = new Option(*iOpt);
    m_options.push_back(opt);
    if(iOpt->name == helpOption().name) helpFound = true;
    m_supportedOptions.insert(std::make_pair(iOpt->flag,opt));
    std::string optNameKey("--");
    m_supportedOptions.insert(std::make_pair(optNameKey.append(iOpt->name),opt));
  }
  if(!helpFound) {
    Option* helpOpt(new Option(helpOption()));
    m_options.push_back(helpOpt);
    m_supportedOptions.insert(std::make_pair(helpOpt->flag,helpOpt));
    std::string optNameKey("--");
    m_supportedOptions.insert(std::make_pair(optNameKey.append(helpOption().name),helpOpt));
  }
  helpFound = false;
  for(std::vector<Command>::const_iterator iCmd = commands.begin(); iCmd!=commands.end(); ++iCmd) {
    Command* cmd = new Command(*iCmd);
    m_commands.push_back(cmd);
    if(iCmd->name == helpOption().name) {
      helpFound = true;
    } else {
      cmd->addOption(helpOption().name);
    }
    m_supportedCommands.insert(std::make_pair(iCmd->flag,cmd));
    std::string cmdNameKey("--");
    m_supportedCommands.insert(std::make_pair(cmdNameKey.append(iCmd->name),cmd));
  }
  if(!helpFound) {
    Command* helpCmd = new Command(helpOption().name);
    helpCmd->flag = helpOption().flag;
    helpCmd->type = helpOption().type;
    helpCmd->helpEntry = helpOption().helpEntry;
    m_commands.push_back(helpCmd);
    m_supportedCommands.insert(std::make_pair(helpCmd->flag,helpCmd));
    std::string cmdNameKey("--");
    m_supportedCommands.insert(std::make_pair(cmdNameKey.append(helpOption().name),helpCmd));
  }
}

coral::CommandLine::~CommandLine(){
  for( std::vector<Option*>::const_iterator iOpt=m_options.begin();
       iOpt!=m_options.end();
       ++iOpt) {
    delete *iOpt;
  }
  for( std::vector<Command*>::const_iterator iCmd=m_commands.begin();
       iCmd!=m_commands.end();
       ++iCmd) {
    delete *iCmd;
  }
}

bool
coral::CommandLine::parse(int argc,char** argv,
                          std::map<std::string,std::string>& userOptions){
  std::stringstream sout;
  std::string currentOpt;
  if(argc<=1) return false;
  for(int i=1; i<argc; i++) {
    std::string singleArg(argv[i]);
    if(argv[i][0]==optionMarker()) {
      if(singleArg.size()==1) {
        throw OptionParseException("Parsing error: Unbound marker '-' found.");
      }
      if(!currentOpt.empty()) {
        userOptions.insert(std::make_pair(currentOpt,sout.str()));
      }
      currentOpt = singleArg;
      sout.str("");
    } else {
      if(currentOpt.empty()) {
        std::stringstream msg;
        msg << "Parsing error: Unbound parameter " << singleArg << " found.";
        throw OptionParseException(msg.str());
      }
      if(!sout.str().empty()) sout << " ";
      sout << singleArg;
    }
    if(i+1==argc && !currentOpt.empty()) {
      userOptions.insert(std::make_pair(currentOpt,sout.str()));
    }
  }
  return true;
}

bool
coral::CommandLine::validate(const std::map<std::string,std::string>& userOptions,
                             const std::map<std::string,Option*>& supportedOptions){
  bool foundExclusive = false;
  Option* firstExclusive=0;
  std::string helpKey("--");
  bool helpFound = (userOptions.find(helpOption().flag)!=userOptions.end()) ||
    (userOptions.find(helpKey.append(helpOption().name))!=userOptions.end());
  for(std::map<std::string,std::string>::const_iterator map_iter = userOptions.begin();
      map_iter!=userOptions.end();
      map_iter++) {
    std::string theOption =  (*map_iter).first;
    std::string theParameterValue = (*map_iter).second;
    std::map<std::string,Option*>::const_iterator opt = supportedOptions.find(theOption);
    if(opt==supportedOptions.end()) {
      throw coral::InvalidOptionException(theOption);
    }
    if(!helpFound) {
      // check exclusive options
      if( opt->second->exclusive) {
        if(!foundExclusive) {
          foundExclusive = true;
          firstExclusive = opt->second;
        } else {
          throw coral::ExclusiveOptionException(firstExclusive->name,opt->second->name);
        }
      }
      if(opt->second->type==Option::BOOLEAN) {
        if(!theParameterValue.empty()) {
          std::stringstream msg;
          msg << "Parameter not expected for option " << theOption << ".";
          throw OptionParseException(msg.str());
        } else {
          theParameterValue = "";
        }
      }
      if(opt->second->type==Option::STRING && theParameterValue.empty()) {
        std::stringstream msg;
        msg << "Parameter required for option " << theOption << " is missing.";
        throw OptionParseException(msg.str());
      }
    }
    m_userOptions.insert(std::make_pair(opt->second->name,theParameterValue));
  }
  if(helpFound) {
    return true;
  }
  for(std::map<std::string,Option*>::const_iterator reqIOpt=supportedOptions.begin();
      reqIOpt!=supportedOptions.end();
      reqIOpt++) {
    if(reqIOpt->second->required) {
      std::map<std::string,std::string>::const_iterator optIter = userOptions.find(reqIOpt->second->flag);
      if(optIter==userOptions.end()) {
        std::string optKey("--");
        optIter = userOptions.find(optKey.append(reqIOpt->second->name));
        if(optIter==userOptions.end()) throw coral::MissingRequiredOptionException(reqIOpt->second->name);
      }
    }
  }
  return true;
}

bool
coral::CommandLine::parse(int argc,char** argv){
  // fill user option map
  if(argc>=1) {
    m_commandName = argv[0];
  }
  m_userOptions.clear();
  std::map<std::string,std::string> userOps;
  if(!parse(argc, argv, userOps)) {
    return false;
  }
  bool ret = false;
  bool commandMode = m_commands.size()>0;
  if(commandMode) {
    Command* helpCmd = 0;
    Command* firstCmd = 0;
    Command* secondCmd = 0;
    for(std::map<std::string,std::string>::const_iterator uOpt = userOps.begin();
        uOpt!=userOps.end();
        ++uOpt) {
      std::map<std::string,Command*>::const_iterator iCmd= m_supportedCommands.find(uOpt->first);
      if(iCmd!=m_supportedCommands.end()) {
        if(iCmd->second->name==helpOption().name) {
          helpCmd = iCmd->second;
        } else {
          if(!firstCmd) {
            firstCmd=iCmd->second;
          } else {
            secondCmd = iCmd->second;
            break;
          }
        }
      } else {
        std::map<std::string,Option*>::const_iterator iOpt= m_supportedOptions.find(uOpt->first);
        if(iOpt==m_supportedOptions.end()) throw coral::InvalidOptionException(uOpt->first);
      }

    }
    if(helpCmd && !firstCmd) {
      firstCmd = helpCmd;
    }

    if(!firstCmd) {
      throw MissingRequiredOptionException();
    } else {
      if(secondCmd) {
        throw ExclusiveOptionException(firstCmd->name,secondCmd->name);
      } else {
        m_userCommand = firstCmd->name;
        std::map<std::string,Option*> commandOptions;
        std::vector<Option*> options;
        Option* cmdOpt(new Option(*firstCmd));
        options.push_back(cmdOpt);
        commandOptions.insert(std::make_pair(cmdOpt->flag,cmdOpt));
        std::string optKey("--");
        commandOptions.insert(std::make_pair(optKey.append(cmdOpt->name),cmdOpt));
        for(std::map<std::string,bool>::const_iterator iDep = firstCmd->dependentOptions().begin();
            iDep!=firstCmd->dependentOptions().end();
            ++iDep) {
          std::string optKey2("--");
          optKey2.append(iDep->first);
          std::map<std::string,Option*>::const_iterator iOpt = m_supportedOptions.find(optKey2);
          if(iOpt!=m_supportedOptions.end()) {
            Option* opt = new Option(*(iOpt->second));
            opt->required = iDep->second;
            options.push_back(opt);
            commandOptions.insert(std::make_pair(optKey2,opt));
            commandOptions.insert(std::make_pair(opt->flag,opt));
          }
        }
        ret = validate(userOps,commandOptions);
        for( std::vector<Option*>::const_iterator iO=options.begin();
             iO!=options.end();
             ++iO) {
          delete *iO;
        }
      }
    }
  } else {
    // check collected options
    ret = validate(userOps,m_supportedOptions);
  }
  return ret;
}

const std::string&
coral::CommandLine::commandName() const {
  return m_commandName;
}

const std::string&
coral::CommandLine::userCommand() const {
  return m_userCommand;
}

const std::map<std::string,std::string>&
coral::CommandLine::userOptions() const {
  return m_userOptions;
}

bool coral::CommandLine::find( const std::string& optionName) const {
  std::map<std::string,std::string>::const_iterator iO=m_userOptions.find(optionName);
  bool found = (iO != m_userOptions.end());
  if(found) {
    m_currentOption.first = iO->first;
    m_currentOption.second = &iO->second;
  }
  return found;
}

const std::string& coral::CommandLine::optionValue( const std::string& optionName ) const {
  const std::string* val = 0;
  if(optionName==m_currentOption.first) {
    val = m_currentOption.second;
  } else {
    std::map<std::string,std::string>::const_iterator iO=m_userOptions.find(optionName);
    bool found = (iO != m_userOptions.end());
    if( ! found ) throw OptionException("Option \""+optionName+"\" has not been selected.");
    val = &iO->second;
  }
  return *val;
}

void coral::CommandLine::printOption(const Option& opt,
                                     std::ostream& out) const {
  if(!opt.required) out << "[";
  out << opt.flag;
  if(opt.type==Option::STRING) out << " "<<opt.name;

  if(!opt.required) out << "]";
  out << " ";
}

void coral::CommandLine::printCommand(const Command& cmd,
                                      std::ostream& out) const {
  Option cmdOpt(cmd);
  cmdOpt.required = true;
  printOption(cmdOpt,out);
  std::vector<Option> reqOps;
  std::vector<Option> nonReqOps;
  for(std::map<std::string,bool>::const_iterator iOp=cmd.dependentOptions().begin();
      iOp!=cmd.dependentOptions().end();
      ++iOp) {
    std::string optKey("--");
    std::map<std::string,Option*>::const_iterator iOpDesc=m_supportedOptions.find(optKey.append(iOp->first));
    if(iOpDesc!=m_supportedOptions.end()) {
      if(iOpDesc->second->name!=helpOption().name) {
        Option opt(*(iOpDesc->second));
        opt.required = iOp->second;
        // separate the required options
        if(opt.required) {
          reqOps.push_back(opt);
        } else{
          nonReqOps.push_back(opt);
        }
      }
    }
  }
  if(cmd.name!=helpOption().name) nonReqOps.push_back(helpOption());
  for(std::vector<Option>::const_iterator iO=reqOps.begin(); iO!=reqOps.end(); ++iO) {
    printOption(*iO,out);
  }
  for(std::vector<Option>::const_iterator iO=nonReqOps.begin(); iO!=nonReqOps.end(); ++iO) {
    printOption(*iO,out);
  }
  out << std::endl;
}


// displays the available options defined
void coral::CommandLine::help(std::ostream& out) const {
  if(m_commandName=="") {
    out << "No command provided." << std::endl;
  } else {
    out << "Usage:" << std::endl;
    if(m_commands.size()>0) {
      out << "        " << m_commandName << " [COMMAND] [PARAMETER_0] ... [PARAMETER_N]"<<std::endl;
      out << std::endl;
      out << "Commands:"<<std::endl;
      for(std::vector<Command*>::const_iterator iter=m_commands.begin(); iter!=m_commands.end(); iter++) {
        std::map<std::string,Command*>::const_iterator iC=m_supportedCommands.find((*iter)->flag);
        if(iC!=m_supportedCommands.end()) {
          out << "        " << (*iter)->flag << " ( " << (*iter)->name << " ) : " << (*iter)->helpEntry << std::endl;
          out << "        ";
          printCommand(*(*iter),out);
          out << std::endl;
        }
      }
    } else {
      out << "        " << m_commandName << " [PARAMETER_0] ... [PARAMETER_N]"<<std::endl;
      std::vector<Option> reqOps;
      std::vector<Option> nonReqOps;
      for(std::vector<Option*>::const_iterator iter=m_options.begin(); iter!=m_options.end(); iter++) {
        std::map<std::string,Option*>::const_iterator iO=m_supportedOptions.find((*iter)->flag);
        if(iO!=m_supportedOptions.end()) {
          // split the required options
          if((*iter)->name!=helpOption().name) {
            if((*iter)->required) {
              reqOps.push_back(*(*iter));
            } else {
              nonReqOps.push_back(*(*iter));
            }
          }
        }
      }
      nonReqOps.push_back(helpOption());
      out << "        ";
      for(std::vector<Option>::const_iterator iO=reqOps.begin(); iO!=reqOps.end(); ++iO) {
        printOption(*iO,out);
      }
      for(std::vector<Option>::const_iterator iO=nonReqOps.begin(); iO!=nonReqOps.end(); ++iO) {
        printOption(*iO,out);
      }
    }
    out << "Parameters:"<<std::endl;
    for(std::vector<Option*>::const_iterator iter=m_options.begin(); iter!=m_options.end(); iter++) {
      std::map<std::string,Option*>::const_iterator iF=m_supportedOptions.find((*iter)->flag);
      if(iF!=m_supportedOptions.end()) {
        out << "        "<< (*iter)->flag << " ( " << (*iter)->name << " ) : " << (*iter)->helpEntry << std::endl;
      }
    }
  }
}

void
coral::CommandLine::help(const std::string& command, std::ostream& out) const {
  std::string optKey("--");
  std::map<std::string,Command*>::const_iterator iC=m_supportedCommands.find(optKey.append(command));
  if(iC!=m_supportedCommands.end()) {
    out << "Usage:" << std::endl;
    out << "        " << m_commandName << " [COMMAND] [PARAMETER_0] ... [PARAMETER_N]"<<std::endl;
    out << std::endl;
    out << "Command:"<<std::endl;
    out << "        " << iC->second->flag << " ( " << iC->second->name << " ) : " << iC->second->helpEntry << std::endl;
    out << "        ";
    printCommand(*(iC->second),std::cout);
    out << "Parameters:"<<std::endl;
    for(std::vector<Option*>::const_iterator iter=m_options.begin(); iter!=m_options.end(); iter++) {
      std::map<std::string,bool>::const_iterator iDep = iC->second->dependentOptions().find((*iter)->name);
      if(iDep!=iC->second->dependentOptions().end()) {
        out << "        " << (*iter)->flag << " ( " << (*iter)->name << " ) : " << (*iter)->helpEntry << std::endl;
      }
    }
  } else {
    help(out);
  }
}

void
coral::CommandLine::dumpUserOptions(std::ostream& out) const {
  std::map<std::string,std::string>::const_iterator iter;
  out << "Options stored = " << m_userOptions.size() << std::endl;
  if(m_userOptions.size()==0) {
    out << "No option specified." << std::endl;
  }
  for(iter = m_userOptions.begin(); iter!=m_userOptions.end(); iter++) {
    out << "option [" << (*iter).first << "] - value=" << (*iter).second << std::endl;
  }
}
