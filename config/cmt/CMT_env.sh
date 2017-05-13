# $Id: CMT_env.sh,v 1.34 2012/02/08 18:41:24 avalassi Exp $

# Check that CMTCONFIG was set before executing this script
if [ "$CMTCONFIG" == "" ] ; then
  echo "ERROR! CMTCONFIG was not set"
  return 1
fi
export CMTCONFIG

# Check that this script is being sourced in its own directory 
if [ ! -e CMT_env.sh ] ; then
  echo "ERROR! Please source CMT_env.sh in its own directory"
  return 1
fi
thePwd=`pwd`

# Set the CMT version
export CMTVERS=v1r20p20090520
###export CMTVERS=v1r21

# Use CVMSFS  (LHCb or SFT) if file 'usecvmfs' is found in this directory
# Set or unset AFS, SITEROOT, CMTROOT, CMTSITE, CMTPROJECTPATH accordingly
# [see /cvmfs/lhcb.cern.ch/group_login.csh for the cvmfs-based LHCb setup]
# Note: /opt/(sft,lhcb) are NO LONGER symlinks to /cvmfs/(sft,lhcb).cern.ch
if [ -e usecvmfs ] && [ -d /cvmfs ]; then
  echo "Configure CMT using CVMFS"
  unset AFS
  export CMTSITE=LOCAL
  if grep -q -i lhcb usecvmfs; then
    echo "Configure CMT using /cvmfs/lhcb.cern.ch"
    export SITEROOT=/cvmfs/lhcb.cern.ch/lib
    export CMTROOT=/cvmfs/lhcb.cern.ch/lib/contrib/CMT/$CMTVERS
    export CMTPROJECTPATH=$SITEROOT/lcg/external
  elif grep -q -i sft usecvmfs; then
    echo "Configure CMT using /cvmfs/sft.cern.ch"
    export SITEROOT=/cvmfs/sft.cern.ch
    export CMTROOT=/cvmfs/sft.cern.ch/lcg/external/CMT/$CMTVERS
    export CMTPROJECTPATH=$SITEROOT/lcg/external
  else
    echo "Configure CMT using /opt/sw and /cvmfs/lhcb.cern.ch"
    export SITEROOT=/cvmfs/lhcb.cern.ch/lib
    export CMTROOT=/cvmfs/lhcb.cern.ch/lib/contrib/CMT/$CMTVERS
    export CMTPROJECTPATH=/opt/sw/lcg/app/releases
  fi
else
  echo "Configure CMT using AFS"
  export AFS=/afs
  unset CMTSITE
  export SITEROOT=/afs/cern.ch
  export CMTROOT=/afs/cern.ch/sw/contrib/CMT/$CMTVERS
  export CMTPROJECTPATH=$SITEROOT/sw/lcg/app/releases
fi

# Setup CMT
. $CMTROOT/mgr/setup.sh

# Set VERBOSE (verbose build for CMT v1r20p2008xxxx or higher)
export VERBOSE=1

# Unset LD_LIBRARY_PATH
unset LD_LIBRARY_PATH

# Unset CMTPATH (use CMTPROJECTPATH)
unset CMTPATH

# Set CMTUSERCONTEXT
export CMTUSERCONTEXT=${thePwd}/USERCONTEXT/${USER}

# Setup icc compiler
if [ "$CMTCONFIG" == "i686-slc5-icc11-dbg" ] || \
   [ "$CMTCONFIG" == "i686-slc5-icc11-opt" ] || \
   [ "$CMTCONFIG" == "x86_64-slc5-icc11-dbg" ] || \
   [ "$CMTCONFIG" == "x86_64-slc5-icc11-opt" ]; then
  coralhome=`cmt show macro_value CORAL_home`
  if [ -e $coralhome/../src/config/cmt/icc_setup.sh ]; then
    . $coralhome/../src/config/cmt/icc_setup.sh
  else
    echo "WARNING! icc_setup.sh not found in $coralhome/../src/config/cmt"
  fi 
fi

# User-specific customizations
if [ -e ${CMTUSERCONTEXT}/CMT_userenv.sh ] ; then
  echo "[Customize user environment using ${CMTUSERCONTEXT}/CMT_userenv.sh]"
  . ${CMTUSERCONTEXT}/CMT_userenv.sh
  echo "[Customize user environment: done]"
fi

# Printout CMT environment variables
echo "CMTCONFIG set to '${CMTCONFIG}'"
echo "CMTROOT set to '$CMTROOT'"
echo "CMTPATH set to '$CMTPATH'"
echo "CMTPROJECTPATH set to '$CMTPROJECTPATH'"
echo "CMTUSERCONTEXT set to '$CMTUSERCONTEXT'"
echo "CMTINSTALLAREA set to '$CMTINSTALLAREA'"
echo "CMTSITE set to '$CMTSITE'"
echo "LD_LIBRARY_PATH set to '$LD_LIBRARY_PATH'"

