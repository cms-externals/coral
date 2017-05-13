# Go to the qmtest directory
cd `dirname ${0}`

# Go to the cmt directory and setup cmt
cd ../cmt
source CMT_env.csh
source setup.csh

# Echo QMTEST_CLASS_PATH
###cmt show macro CMTINSTALLAREA
###cmt show set QMTEST_CLASS_PATH
echo "Using QMTEST_CLASS_PATH=$QMTEST_CLASS_PATH"

# Set a few additional environment variables
###setenv CORAL_AUTH_PATH ${HOME}/private
###setenv CORAL_DBLOOKUP_PATH ${LOCALRT}/src/RelationalCool/tests
setenv CORAL_AUTH_PATH /afs/cern.ch/sw/lcg/app/pool/db
setenv CORAL_DBLOOKUP_PATH /afs/cern.ch/sw/lcg/app/pool/db

# Go back to the qmtest directory
cd ../qmtest

# Create the qmtest test suites
./createconfig.py

# Define the qmtest results file
set theQmr=${CMTCONFIG}.qmr

# List all tests
echo "*** LIST TESTS *********************************************************"
echo "qmtest ls"
###qmtest ls

# Print out some examples
echo "*** EXAMPLES ***********************************************************"
echo "qmtest run"
echo "qmtest run integration.monitoring_oracle-oracle"
