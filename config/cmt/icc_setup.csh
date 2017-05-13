#!/usr/local/bin/tcsh -f

set argv0=icc_setup.csh

if ( "$1" != "" ) then
  echo "Usage: source $argv0"
  exit 1
endif

###set intelplat=$1 # (intel64|ia32)
###set intel_home=/afs/cern.ch/sw/IntelSoftware/linux
###set icc_c_home=/afs/cern.ch/sw/IntelSoftware/linux/x86_64/Compiler/11.1/072
###set icc_f_home=/afs/cern.ch/sw/IntelSoftware/linux/x86_64/Compiler/11.1/072
set intelplat=`cmt show set_value intelplat`
set intel_home=`cmt show set_value intel_home`
set icc_c_home=`cmt show set_value icc_c_home`
set icc_f_home=`cmt show set_value icc_f_home`

if ( "$intelplat" != "intel64" && "$intelplat" != "ia32" ) then
  echo "ERROR! intelplat=$intelplat (expect ia32 or intel64)"
  exit 1
endif

echo "${argv0}: *** Setup for Intel Compiler Suite started"

if ( "x"`env | grep INTEL_LICENSE_FILE` == "x" ) then
  echo "${argv0}: Sourcing Intel licensing script"
  ###source ${intel_home}/setup.csh
  setenv INTEL_LICENSE_FILE `bash -c '. /afs/cern.ch/sw/IntelSoftware/linux/setup.sh > /dev/null; echo $INTEL_LICENSE_FILE'`
  echo "The environment for Intel software tools for Linux has been set up:"
  echo "License ports: $INTEL_LICENSE_FILE"
else
  echo "${argv0}: Intel license already sourced, skipping this step ..."
endif

###source ${icc_c_home}/bin/iccvars.csh ${intelplat} ${intel_home} ${icc_c_home} ${icc_f_home}
###source ${icc_f_home}/bin/ifortvars.csh ${intelplat} ${intel_home} ${icc_c_home} ${icc_f_home}

echo "${argv0}: Set PATH to Intel compiler"
setenv PATH `echo $PATH | tr ":" "\n" | grep -v $intel_home | tr "\n" ":" | sed 's/:$/\n/'`
setenv PATH ${icc_c_home}/bin/${intelplat}:${PATH}

echo "${argv0}: Set LD_LIBRARY_PATH to Intel compiler"
if ( "${?LD_LIBRARY_PATH}" == 0 ) setenv LD_LIBRARY_PATH ""
setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | tr ":" "\n" | grep -v $intel_home | tr "\n" ":" | sed 's/:$/\n/'`
if ( "$intelplat" == "intel64" ) then
  setenv LD_LIBRARY_PATH ${icc_c_home}/lib/intel64:${icc_c_home}/em64t/sharedlib:${icc_c_home}/mkl/lib/em64t:${icc_c_home}/tbb/em64t/cc4.1.0_libc2.4_kernel2.6.16.21/lib:${LD_LIBRARY_PATH}
else if ( "$intelplat" == "ia32" ) then
  setenv LD_LIBRARY_PATH ${icc_c_home}/lib/ia32:${icc_c_home}/ipp/ia32/sharedlib:${icc_c_home}/mkl/lib/32:${icc_c_home}/tbb/ia32/cc4.1.0_libc2.4_kernel2.6.16.21/lib:${LD_LIBRARY_PATH}
endif
setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | sed 's/:$/\n/'`

echo "${argv0}: *** Setup for Intel Compiler Suite finished"
