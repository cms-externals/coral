#! /bin/bash

# Check the arguments
if [ "$1" = "" ]; then
    theTests=""
    ###theTests=integration
    ###theTests=unit
    ###theTests=unit_coralserver
else
    theTests=""
    while [ "$1" != "" ]; do
        if [ "$theTests" = "" ]; then
            theTests="$1"
        else
            theTests="$theTests $1"
        fi
        shift
    done
fi
echo Will launch \'qmtest run ${theTests}\'

# Go to the qmtest directory
cd `dirname ${0}`

# Workaround for problems with CMTINSTALLAREA: path mismatch between
# /afs/cern.ch/sw/lcg/app/releases/COOL/internal/avalassi/COOL_HEAD/src
# and /afs/cern.ch/user/a/avalassi/myLCG/COOL_HEAD/src...
# This is needed when launching this script from coolBuildCMT.sh,
# otherwise QMTEST_CLASS_PATH is equal to "/src/config/qmtest"...
###export CMTINSTALLAREA=`cd ../../..; pwd`

# Go to the cmt directory and setup cmt
cd ../cmt
###echo "Set up the CMT runtime environment"
source CMT_env.sh > /dev/null 2>&1
###echo "Set up the CORAL runtime environment"
source setup.sh

# Set a few additional environment variables
# (as in prepare_env in test_functions.sh)
###export CORAL_AUTH_PATH=/afs/cern/sw/lcg/app/pool/db
###export CORAL_DBLOOKUP_PATH=/afs/cern/sw/lcg/app/pool/db

# Define the qmtest results file
theQmrDir=`cd ../../logs/qmtest; pwd`
theQmr=${theQmrDir}/${CMTCONFIG}.qmr

# Ignore QMTEST timeouts for COOL
###export COOL_IGNORE_TIMEOUT=yes

# Go to the qmtest directory and check QMTEST_CLASS_PATH (bug #86964)
###cmt show macro CMTINSTALLAREA
###cmt show set QMTEST_CLASS_PATH
cd ../qmtest
echo "Using QMTEST_CLASS_PATH=$QMTEST_CLASS_PATH"
if [ "$QMTEST_CLASS_PATH" != `pwd` ]; then
  echo "ERROR! Expected QMTEST_CLASS_PATH="`pwd`
  exit 1
fi

# Run the tests from the qmtest directory
echo "Launch tests - results will be in ${theQmr}"
echo Launch \'qmtest run ${theTests}\'
qmtest run -o ${theQmr} ${theTests} > /dev/null 2>&1

# Do not attempt to run 'qmtest report/summarize' as "$?" is wrong on OSX?
