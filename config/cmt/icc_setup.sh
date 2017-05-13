#!/bin/sh

argv0=icc_setup.sh
if [ "$1" != "" ]; then
  echo "Usage: . $argv0"
  return 1
fi

###intelplat=$1 # (intel64|ia32)
###intel_home=/afs/cern.ch/sw/IntelSoftware/linux
###icc_c_home=/afs/cern.ch/sw/IntelSoftware/linux/x86_64/Compiler/11.1/072
###icc_f_home=/afs/cern.ch/sw/IntelSoftware/linux/x86_64/Compiler/11.1/072
intelplat=`cmt show set_value intelplat`
intel_home=`cmt show set_value intel_home`
icc_c_home=`cmt show set_value icc_c_home`
icc_f_home=`cmt show set_value icc_f_home`

if [ "$intelplat" != "intel64" -a "$intelplat" != "ia32" ]; then
  echo "ERROR! intelplat=$intelplat (expect ia32 or intel64)"
  exit 1
fi

echo "${argv0}: *** Setup for Intel Compiler Suite started"

if [ x`env | grep INTEL_LICENSE_FILE` = "x" ] ; then
  echo "${argv0}: Sourcing Intel licensing script"
  . ${intel_home}/setup.sh
else
  echo "${argv0}: Intel license already sourced, skipping this step ..."
fi

###. ${icc_c_home}/bin/iccvars.sh ${intelplat} ${intel_home} ${icc_c_home} ${icc_f_home}
###. ${icc_f_home}/bin/ifortvars.sh ${intelplat} ${intel_home} ${icc_c_home} ${icc_f_home}

echo "${argv0}: Set PATH to Intel compiler"
export PATH=`echo $PATH | tr ":" "\n" | grep -v $intel_home | tr "\n" ":" | sed 's/:$/\n/'`
export PATH=${icc_c_home}/bin/${intelplat}:${PATH}

echo "${argv0}: Set LD_LIBRARY_PATH to Intel compiler"
export LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | tr ":" "\n" | grep -v $intel_home | tr "\n" ":" | sed 's/:$/\n/'`
if [ "$intelplat" == "intel64" ]; then
  export LD_LIBRARY_PATH=${icc_c_home}/lib/intel64:${icc_c_home}/em64t/sharedlib:${icc_c_home}/mkl/lib/em64t:${icc_c_home}/tbb/em64t/cc4.1.0_libc2.4_kernel2.6.16.21/lib:${LD_LIBRARY_PATH}
elif [ "$intelplat" == "ia32" ]; then
  export LD_LIBRARY_PATH=${icc_c_home}/lib/ia32:${icc_c_home}/ipp/ia32/sharedlib:${icc_c_home}/mkl/lib/32:${icc_c_home}/tbb/ia32/cc4.1.0_libc2.4_kernel2.6.16.21/lib:${LD_LIBRARY_PATH}
fi
export LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed 's/:$/\n/'`

echo "${argv0}: *** Setup for Intel Compiler Suite finished"
