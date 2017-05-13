# $Id: CMT_env.csh,v 1.33 2012/02/08 18:41:24 avalassi Exp $

# Check that CMTCONFIG was set before executing this script
if ( "${?CMTCONFIG}" != "1" ) then
  echo "ERROR! CMTCONFIG was not set"
  exit 1
endif

# Check that this script is being sourced in its own directory 
if ( ! -e CMT_env.csh ) then
  echo "ERROR! Please source CMT_env.csh in its own directory"
  exit 1
endif
set thePwd=`pwd`

# Set the CMT version
setenv CMTVERS v1r20p20090520
###setenv CMTVERS v1r21

# Use CVMSFS  (LHCb or SFT) if file 'usecvmfs' is found in this directory
# Set or unset AFS, SITEROOT, CMTROOT, CMTSITE, CMTPROJECTPATH accordingly
# [see /cvmfs/lhcb.cern.ch/group_login.csh for the cvmfs-based LHCb setup]
# Note: /opt/(sft,lhcb) are NO LONGER symlinks to /cvmfs/(sft,lhcb).cern.ch
if ( -e usecvmfs && -d /cvmfs ) then
  echo "Configure CMT using CVMFS"
  unsetenv AFS /afs
  setenv CMTSITE LOCAL
  if ( { grep -q -i lhcb usecvmfs } ) then
    echo "Configure CMT using /cvmfs/lhcb.cern.ch"
    setenv SITEROOT /cvmfs/lhcb.cern.ch/lib
    setenv CMTROOT /cvmfs/lhcb.cern.ch/lib/contrib/CMT/$CMTVERS
    setenv CMTPROJECTPATH $SITEROOT/lcg/external
  else if ( { grep -q -i sft usecvmfs } ) then
    echo "Configure CMT using /cvmfs/sft.cern.ch"
    setenv SITEROOT /cvmfs/sft.cern.ch
    setenv CMTROOT /cvmfs/sft.cern.ch/lcg/external/CMT/$CMTVERS
    setenv CMTPROJECTPATH $SITEROOT/lcg/external
  else
    echo "Configure CMT using /opt/sw and /cvmfs/lhcb.cern.ch"
    setenv SITEROOT /cvmfs/lhcb.cern.ch/lib
    setenv CMTROOT /cvmfs/lhcb.cern.ch/lib/contrib/CMT/$CMTVERS
    setenv CMTPROJECTPATH /opt/sw/lcg/app/releases
  endif
else
  echo "Configure CMT using AFS"
  setenv AFS /afs
  unsetenv CMTSITE
  setenv SITEROOT /afs/cern.ch
  setenv CMTROOT /afs/cern.ch/sw/contrib/CMT/$CMTVERS
  setenv CMTPROJECTPATH $SITEROOT/sw/lcg/app/releases
endif

# Setup CMT
source $CMTROOT/mgr/setup.csh

# Set VERBOSE (verbose build for CMT v1r20p2008xxxx or higher)
setenv VERBOSE 1

# Unset LD_LIBRARY_PATH
setenv LD_LIBRARY_PATH

# Unset CMTPATH (use CMTPROJECTPATH)
setenv CMTPATH

# Set CMTUSERCONTEXT
setenv CMTUSERCONTEXT ${thePwd}/USERCONTEXT/${USER}

# Setup icc compiler
if ( "$CMTCONFIG" == "i686-slc5-icc11-dbg" || \
     "$CMTCONFIG" == "i686-slc5-icc11-opt" || \
     "$CMTCONFIG" == "x86_64-slc5-icc11-dbg" || \
     "$CMTCONFIG" == "x86_64-slc5-icc11-opt" ) then
  set coralhome=`cmt show macro_value CORAL_home`
  if ( -e $coralhome/../src/config/cmt/icc_setup.csh ) then
    source $coralhome/../src/config/cmt/icc_setup.csh
  else
    echo "WARNING! icc_setup.csh not found in $coralhome/../src/config/cmt"
  endif 
endif

# User-specific customizations
if ( -e ${CMTUSERCONTEXT}/CMT_userenv.csh ) then
  echo "[Customize user environment using ${CMTUSERCONTEXT}/CMT_userenv.csh]"
  source ${CMTUSERCONTEXT}/CMT_userenv.csh
  echo "[Customize user environment: done]"
endif

# Printout CMT environment variables
echo "CMTCONFIG set to '$CMTCONFIG'"
echo "CMTROOT set to '$CMTROOT'"
echo "CMTPATH set to '$CMTPATH'"
echo "CMTPROJECTPATH set to '$CMTPROJECTPATH'"
echo "CMTUSERCONTEXT set to '$CMTUSERCONTEXT'"
if ( "${?CMTINSTALLAREA}" == "1" ) then
  echo "CMTINSTALLAREA set to '$CMTINSTALLAREA'"
else
  echo "CMTINSTALLAREA is not set"
endif
if ( "${?CMTSITE}" == "1" ) then
  echo "CMTSITE set to '$CMTSITE'"
else
  echo "CMTSITE is not set"
endif
echo "LD_LIBRARY_PATH set to '$LD_LIBRARY_PATH'"
