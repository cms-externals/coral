test="HLT"

if [ "$1" == "" ] || [ "$2" != "" ]; then
  echo "ERROR! Unexpected arguments for ATLAS_CMT_env.sh"
  echo "Usage: . ATLAS_CMT_env.sh X.Y.Z"
  return 1
else
  export REL=$1
  shift
fi

if [ ! -e ATLAS_CMT_env.sh ]; then
  echo "ERROR! Please execute ATLAS_CMT_env.sh in its own directory"
  return 1
fi
export CORAL_SERVER_TESTS_HLT=`cd ..; pwd` # For macro ATLAS_TEST_AREA

# Check if the OS is supported (AtlasSetup will choose CMTCONFIG by itself)
slc=`more /etc/redhat-release | tr "\." " " | awk '{print $4$6}'`
if [ "$slc" == "SLC4" ]; then
  echo "Host O/S is $slc"
  echo "ERROR! SLC4 is no longer supported!"
  exit 1
elif [ "$slc" == "SLC5" ]; then
  echo "Host O/S is $slc"
else
  echo "ERROR! Unknown Redhat release '"`more /etc/redhat-release`"'"
  exit 1
fi

unset CMTCONFIG
unset CMTPATH
unset CMTPROJECTPATH
unset CMTSITE
unset CMTUSERCONTEXT

export AFS=/afs
export SITEROOT=$AFS/cern.ch
export CMTROOT=$SITEROOT/sw/contrib/CMT/v1r22
. $CMTROOT/mgr/setup.sh

echo "*** Setup the ATLAS software environment for release $REL ..."
echo "CMTCONFIG (before AtlasSetup) is $CMTCONFIG"
export AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup
log=`mktemp`
dbg='opt' # choose 'opt' (default) or 'dbg'
. $AtlasSetup/scripts/asetup.sh AtlasHLT,$REL,$dbg >$log 2>&1
###. $AtlasSetup/scripts/asetup.sh AtlasHLT,$REL,32,$dbg >$log 2>&1
###. $AtlasSetup/scripts/asetup.sh AtlasHLT,$REL,64,$dbg >$log 2>&1
cat $log | grep -v "dqm-common requested with conflicting releases" 
if [ "$AtlasVersion" == "" ]; then
  echo "ERROR! AtlasVersion is not set?!"
  return 1
elif [ "$AtlasVersion" != "$REL" ]; then
  echo "ERROR! AtlasVersion ($AtlasVersion) is not equal to REL ($REL)?!"
  return 1
else
  echo "AtlasVersion=$AtlasVersion"
fi
echo "CMTCONFIG (from AtlasSetup) is $CMTCONFIG"
echo "*** Setup the ATLAS software environment for release $REL ... done"

# Workaround for bug #76310 (ATLAS ReplicaSorter skips Oracle replicas in HLT)
if [ "$REL" == "16.1.0" ]; then
  atlasLibs=/afs/cern.ch/sw/lcg/app/releases/CORAL/internal/ATLAS/16.1.0-patches/i686-slc5-gcc43-opt/lib
  echo "LD_LIBRARY_PATH=$atlasLibs:\$LD_LIBRARY_PATH"   
  export LD_LIBRARY_PATH=$atlasLibs:$LD_LIBRARY_PATH    
fi
