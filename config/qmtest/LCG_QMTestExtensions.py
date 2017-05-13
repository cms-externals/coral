########################################################################
# File:   LCG_QMTestExtensions.py
# Author: Manuel Gallas CERN/PH-SFT 
# Date:   14/11/2005   
#
# Contents:
"""  Extension test-classes for QMTest test framework 
   (http://www.codesourcery.com/qmtest/). 

    - Using (at least) QMTest2.3 and Python-2.4.2 

"""  
#                                                                    
# This code is free software; you can redistribute it and/or       
# modify it under the terms of the GNU General Public License         
# as published by the Free Software Foundation; either version 2      
# of the License, or (at your option) any later version.              
#                                                                    
# This library is distributed in the hope that it will be useful,     
# but WITHOUT ANY WARRANTY; without even the implied warranty of      
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU    
# General Public License for more details.         
#
########################################################################
__author__ = 'M. Gallas CERN/PH-SFT' 
__version__ ="$Revision: 1.15 $" 


########################################################################
# Imports  
########################################################################
import fileinput, logging, sys, os, shutil
import qm
from   qm.test.classes.command import ExecTestBase 
import re

########################################################################
# Loggers  
########################################################################
console = logging.StreamHandler(sys.stdout)
formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
console.setFormatter(formatter)
logger=logging.getLogger('LCG_QMTestExtensions')
logger.addHandler(console)
logger.setLevel(20)

########################################################################
# Classes 
########################################################################
class ExecTestBase2(ExecTestBase):
    """  This class inherits from the QMTest ExecTestBase class and 
       instead of the default stdout check it does a 'smart' comparision
       for the tagged lines.  
       - The tag can be selected by the user in the test-case 
         description.
       - In case the line has '=' it will compare both sides of the '=' 
         and in case of an integer or float in rigth side it can operate
         with a % of tolerance.        
    """
    # extra needed arguments 
    arguments=[
          qm.fields.TextField(
             name="stdout_tag",
             title="Standard Output tag",
             verbatim="true",
             multiline="false",
             description="""The stdout will be compared on those
                            lines with the given tag."""),
          qm.fields.TextField(
             name="stdout_ref_path",
             title="Standard Output reference file path ",
             verbatim="true",
             multiline="false",
             description="""The stdout will be compared on those
                            lines with the given tag taking as a 
                            reference the file here pointed.""",
             default_value=''),
          qm.fields.TextField(
             name="stdout_ref",
             title="Standard Output reference file",
             verbatim="true",
             multiline="false",
             description="""The stdout will be compared on those
                            lines with the given tag taking as a 
                            reference the file here pointed.""",
             default_value=''),
          qm.fields.TextField(
             name="excluded_lines",
             title="Excluded lines in the standard Output",
             verbatim="true",
             multiline="true",
             description="""The stdout will be compared on those
                            lines with the given tag and excluding 
                            the lines here described.""",  
             default_value=''),
          qm.fields.TextField(
             name="description_test",
             title="Description of the test",
             verbatim="true",
             multiline="true",
             description="""This field contains the description of the 
                            test (optional).""",
             default_value=''),
          qm.fields.IntegerField(
            name="stdout_tol",
            title="Tolerance",
            description="""The tolerance  permitted at the time 
                           integers or floats are compared.  """,
            default_value = 0),
          qm.fields.TextField(
             name="unsupported_platforms",
             title="",
             verbatim="true",
             multiline="true",
             description="""Comma-separated list of regular expression matching the platforms on which the test cannot be executed.""",  
             default_value=''),
        ]
    def ValidateOutput(self, stdout, stderr, result):
        """Validate the output of the program. No check is done for the
           'stdout'

        'stdout' -- A string containing the data written to the 
                    standard output stream.

        'stderr' -- A string containing the data written to the 
                    standard error stream.

        'result' -- A 'Result' object. It may be used to annotate
                    the outcome according to the content of stderr.

        returns -- A list of strings giving causes of failure."""
        # Store .ref and .log files in a platform-specific subdirectory
        # (avoid possible clashes if several platforms are tested)
        if "CMTCONFIG" in os.environ:
            try: os.mkdir( os.environ['CMTCONFIG'] )
            except OSError: pass
            stdout_log_path=os.environ['CMTCONFIG']+os.sep
        else:
            stdout_log_path=''
        # Maybe some verbosity is needed here
        if not(self.stdout_tag==''):
            strlog='the tag is ' + self.stdout_tag
            logger.debug('ExecTestBase2:ValidateOutput: '+strlog)
        if not(self.stdout_tol==0):
            strlog='the tolerance is ' + repr(self.stdout_tol) 
            logger.debug('ExecTestBase2:ValidateOutput: '+strlog)
        if not(self.stdout_ref==''):
            if not(self.stdout_ref_path==''):
               self.reference_file=self.stdout_ref_path+os.sep+self.stdout_ref 
            else:
               self.reference_file=self.stdout_ref 
            if os.path.abspath(self.reference_file) != os.path.abspath(stdout_log_path+str(self.stdout_ref)):
                shutil.copyfile(os.path.abspath(self.reference_file),
                                stdout_log_path+str(self.stdout_ref))
            ref_file_stdout=''
            for l in fileinput.input(stdout_log_path+str(self.stdout_ref)): 
                ref_file_stdout=ref_file_stdout+l.strip()+'\n'
        if not(self.excluded_lines==''):
            strlog='the excluded lines are ' + self.excluded_lines 
            logger.debug('ExecTestBase2:ValidateOutput: '+strlog) 
        # Copy the log for later use as ref 
        f_ouput=open(stdout_log_path+self.stdout_ref.rstrip('ref')+'log', 'w')
        f_ouput.write(stdout)
        f_ouput.close()
        # Check to see if the standard output matches.
        self.causes = []
        if not(self.stdout=='*'):
            if not(self.stdout_ref==''):
                #  the reference output is described in a
                # external reference file 
                if not self.__CompareText1(stdout, ref_file_stdout, result):
                    self.causes.append("standard output") 
                    result["ExecTest.expected_stdout"] = result.Quote(self.stdout)
            else:
                # the reference output is described in the test-case
                if not self.__CompareText1(stdout, self.stdout,result):
                    self.causes.append("standard output") 
                    result["ExecTest.expected_stdout"] = result.Quote(self.stdout)
        else:
            result["ExecTest.expected_stdout"] = result.Quote(self.stdout)
            
        # Check to see if the standard error matches.
        if not(self.stderr=='*'):
            if not self.__CompareText(stderr, self.stderr):
                self.causes.append("standard error")
                result["ExecTest.expected_stderr"] = result.Quote(self.stderr)
        else: 
            result["ExecTest.expected_stderr"] = result.Quote(self.stderr)
        #
        return self.causes

    def __CompareText(self, s1, s2):
        """Compare 's1' and 's2', ignoring line endings.

        's1' -- A string.

        's2' -- A string.

        returns -- True if 's1' and 's2' are the same, ignoring
        differences in line endings.
         
        This is the standard mode in which QMTest works """
        # The "splitlines" method works independently of the line ending
        # convention in use.
        return s1.splitlines() == s2.splitlines()

    def __CompareText1(self, s1, s2,result):
        """Compare 's1' and 's2', ignoring line endings.

        's1' -- A string.

        's2' -- A string. (is the reference)  

        returns -- True if 's1' and 's2' are the 'same' (for int and float
                  there is a tolerance range in %), ignoring differences 
                  in line endings and those lines without the tag for 
                  comparison .

          The "splitlines" method works independently of the line ending
        convention in use.

          The strings are filtered looking for the 'tag' and the leading 
        and traling whitespaces removed. Scan of the s1 and ref_s1=s2
        """
        # lines with tag that are excluded by hand (by the user)  
        s0_excluded=list()
        for l0 in self.excluded_lines.splitlines():
            s0_excluded.append(l0.strip()) 

        s1_filtered=list()
        s2_filtered=list() 
        for l1 in s1.splitlines(): 
            if l1.__contains__(self.stdout_tag): 
                check=0
                for k in s0_excluded: 
                    if l1.__contains__(k): check=check+1 
                if check==0: s1_filtered.append(l1.strip())
        for l2 in s2.splitlines(): 
            if l2.__contains__(self.stdout_tag): 
                check=0
                for k in s0_excluded: 
                    if l2.__contains__(k): check=check+1 
                if check==0: s2_filtered.append(l2.strip())
        # some debug: shows the lines which will be compared 
        mm=''
        nTot=0
        nDif=0
        nMis=0
        for l in range(max(len(s1_filtered),len(s2_filtered))): 
            nTot=nTot+1
            if  ( l>len(s1_filtered)-1 ): # ref[l] exists but log[l] does not
                nMis=nMis+1
                if ( nMis<=5 ) : # print only the first 5 missing lines
                    mm=mm+'\n%5i'%l
                    if ( nMis<5 ) :
                        mm=mm+'-> log: ...MISSING('+repr(nMis)+')...'+'\n' 
                    else:
                        mm=mm+'-> log: ...MISSING(5)... SKIP THE REST'+'\n' 
                    if(l<=len(s2_filtered)-1):
                        mm=mm+'        ref: '+s2_filtered[l]+'\n'
                    else:
                        mm=mm+'        ref: '+'\n'
            elif( l>len(s2_filtered)-1 or # log[l] exists but ref[l] does not
                  s1_filtered[l] != s2_filtered[l] ): # log[l] != ref[l]
                nDif=nDif+1
                mm=mm+'\n%5i'%l
                mm=mm+'-> log: '+s1_filtered[l]+'\n'
                if(l<=len(s2_filtered)-1):
                    mm=mm+'        ref: '+s2_filtered[l]+'\n' 
                else:
                    mm=mm+'        ref: '+'\n' 
        if(nDif>0 or nMis>0): mm=mm+'\nSummary: '+repr(nDif)+' lines differ and '+repr(nMis)+' are missing out of '+repr(nTot)+' lines\n'
        result["ExecTest.stdout_VS_expected_stdout"] = result.Quote(mm)
        logger.debug('ExecTestBase2:__CompareTest1: '+mm) 
        # Comparision of filtered sets 
        # - filtered sets should have the same length: if this is not the 
        #   case the test will stop here 
        if not(len(s1_filtered)==len(s2_filtered)): 
           self.causes.append(' Different number of tagged lines to compare \n'+\
                              'in the stdout and ref_stdout')
           return False 
        # Scan of the s1 and ref_s1=s2 looking for the '=' 
        s1_filtered_equals=list()
        s2_filtered_equals=list() 
        for i in range(len(s1_filtered)):
            if s1_filtered[i].__contains__('='):
                s1_filtered_equals.append(s1_filtered[i].replace(\
                                                    self.stdout_tag,'').strip())
            if s2_filtered[i].__contains__('='):
                s2_filtered_equals.append(s2_filtered[i].replace(\
                                                    self.stdout_tag,'').strip())  
        #   - in case there is not '=' the strings have to be the same 
            if(not(s1_filtered[i].__contains__('=')) and 
               s2_filtered[i].__contains__('=')): return False   
            if(not(s1_filtered[i].__contains__('=')) and 
               not(s2_filtered[i].__contains__('=')) and 
               not(s1_filtered[i]==s2_filtered[i])): return False 

        # Analysis of lines with '='
        fail_cond=True 
        logger.debug('ExecTestBase2:__CompareTest1: # lines with = is '+\
                     repr(len(s1_filtered_equals))) 
        for i in range(len(s1_filtered_equals)):
            s1_split_list=s1_filtered_equals[i].split('=')
            s2_split_list=s2_filtered_equals[i].split('=')
            logger.debug('ExecTestBase2:__CompareTest1: right side of = for '+\
                          repr(i)+' are '+s1_split_list[1]+' '+s2_split_list[1])
            # - No local tolerance marked with '+-' in the s2
            if not(s2_split_list[1].__contains__('+-')):
                try:    # integer and float to float
                    s1_split_list_1=float(s1_split_list[1].strip())
                    s2_split_list_1=float(s2_split_list[1].strip())
                        #  - comparison with global tolerance (if any) 
                    if(s1_split_list_1!=0.): 
                      if(not(s1_split_list[0]==s2_split_list[0]) or 
                         not((s1_split_list_1==s2_split_list_1) or 
                             ((s1_split_list_1<s2_split_list_1+\
                                  s2_split_list_1*float(self.stdout_tol)/100) and 
                              (s1_split_list_1>s2_split_list_1-
                                  s2_split_list_1*float(self.stdout_tol)/100)))  
                        ): fail_cond=fail_cond and False 
                    else: # case = 0  
                      if(not(s1_split_list[0]==s2_split_list[0]) or 
                         not((s1_split_list_1==s2_split_list_1) or 
                             ((s1_split_list_1<s2_split_list_1+\
                                  float(self.stdout_tol)/100) and 
                              (s1_split_list_1>s2_split_list_1-
                                  float(self.stdout_tol)/100)))  
                        ): fail_cond=fail_cond and False 
                    logger.debug('ExecTestBase2:__CompareTest1: right side of = for '+\
                                 repr(i)+' are '+repr(s1_split_list_1)+' '+\
                                 repr(s2_split_list_1)+' with global tol (%) '+\
                                 repr(self.stdout_tol)+' '+repr(fail_cond) )
                except: # strings
                    s1_split_list[1]=s1_split_list[1].strip() 
                    s2_split_list[1]=s2_split_list[1].strip() 
                    logger.debug('ExecTestBase2:__CompareTest1: right side of = for '+\
                                  repr(i)+' are '+s1_split_list[1]+' '+\
                                  s2_split_list[1])
                    if(not(s1_split_list[0]==s2_split_list[0]) or 
                       not(s1_split_list[1]==s2_split_list[1]) ): fail_cond=fail_cond and False 
            else: 
                #  - comparison with local tolerance  
                print 'mgallas, to be done local tolerance'
        return  fail_cond

        for j in range(len(self.causes)):
           print 'mgallas causes  '+causes[j]
            
        return True
    

    
class ShellCommandTest(ExecTestBase2):    
    """Check a shell command's output and exit code.

    A 'ShellCommandTest' runs the shell and compares its standard
    output, standard error, and exit code with expected values.  The
    shell may be provided with command-line arguments and/or standard
    input.

    QMTest determines which shell to use by the following method:

      - If the context contains the property
        'ShellCommandTest.command_shell', its value is split into
        an argument list and used.

      - Otherwise, if the '.qmrc' configuration file contains the common
        property 'command_shell', its value is split into an argument
        list and used.

      - Otherwise, the default shell for the target system is used.

    Note: the stdout is not checked, only recorded.  
    """

    arguments = [
        qm.fields.TextField(
            name="command",
            title="Command",
            description="""The arguments to the shell.

            This field contains the arguments that are passed to the
            shell.  It should not contain the path to the shell itself.

            If this field is left blank, the shell is run without
            arguments."""
            )
        ]
    
    def SupportedPlatform(self, context, result):
        platform = self.GetPlatform()
        unsupported = [ re.compile(x.strip()) for x in str(self.unsupported_platforms).split(',') if x.strip() ]
        for p_re in unsupported:
            if p_re.search(platform):
                result.SetOutcome(result.UNTESTED)
                result[result.CAUSE] = 'Platform not supported.'
                return False
        return True    

    def GetPlatform(self):
        """
        Return the platform Id defined in CMTCONFIG or SCRAM_ARCH.
        """
        arch = "None"
        # check architecture name
        if "CMTCONFIG" in os.environ:
            arch = os.environ["CMTCONFIG"]
        elif "SCRAM_ARCH" in os.environ:
            arch = os.environ["SCRAM_ARCH"]
        return arch

    def Run(self, context, result):
        """Run the test.

        'context' -- A 'Context' giving run-time parameters to the
        test.

        'result' -- A 'Result' object.  The outcome will be
        'Result.PASS' when this method is called.  The 'result' may be
        modified by this method to indicate outcomes other than
        'Result.PASS' or to add annotations."""

        # check if the test can be run on the current platform
        if not self.SupportedPlatform(context, result):
            return

        # If the context specifies a shell, use it.
        if context.has_key("ShellCommandTest.command_shell"):
            # Split the context value to build the argument list.
            shell = qm.common.split_argument_list(
                context["ShellCommandTest.command_shell"])
        else:
            # Otherwise, use a platform-specific default.
            shell = qm.platform.get_shell_for_command()

        # Environment for the nightlies (user lcgnight)
        if sys.platform.startswith("win") \
               and "USERNAME" in os.environ \
               and os.environ["USERNAME"] == "lcgnight" :
            # Take TNS_ADMIN from DFS on Win (bug #86450)
            os.environ["TNS_ADMIN"]=r"\\cern.ch\dfs\Applications\Oracle\ADMIN"
            # Use credentials from %USERPROFILE%\private on Win (bug #86463)
            private = os.path.join(os.environ["USERPROFILE"],"private")
            os.environ["CORAL_AUTH_PATH"] = private
            os.environ["CORAL_DBLOOKUP_PATH"] = os.environ["CORAL_AUTH_PATH"]
        elif sys.platform.startswith("darwin") \
               and "USER" in os.environ \
               and os.environ["USER"] == "lcgnight" :
            # Use credentials from $HOME/private on Mac (bug #86463)
            private = os.path.join(os.environ["HOME"],"private")
            os.environ["CORAL_AUTH_PATH"] = private
            os.environ["CORAL_DBLOOKUP_PATH"] = os.environ["CORAL_AUTH_PATH"]
        elif sys.platform.startswith("linux") \
               and "USER" in os.environ \
               and os.environ["USER"] == "lcgnight" :
            # Use credentials from POOL AFS area on Linux (bug #86463)
            os.environ["CORAL_AUTH_PATH"] = "/afs/cern.ch/sw/lcg/app/pool/db"
            os.environ["CORAL_DBLOOKUP_PATH"] = os.environ["CORAL_AUTH_PATH"]

        # Special handling if this is a python script (from COOLTests.py)
        if sys.platform.startswith("win") and \
               self.command.split()[0].endswith(".py"):
            # We need to run a python test and not a standard executable
            [cmd, arg] = self.command.split(' ',1)
            for d in os.environ["PATH"].split(os.path.pathsep):
                if os.path.isdir(d) and cmd in os.listdir(d):
                    # we found the script in the path
                    self.command = 'python "%s" %s'%(os.path.join(d,cmd),arg)
                    break

        # Append the command at the end of the argument list.
        arguments = shell + [ self.command ]
        self.RunProgram(arguments[0], arguments, context, result)        



########################################################################
# Local Variables:
# mode: python
# indent-tabs-mode: nil
# fill-column: 72
# End:
