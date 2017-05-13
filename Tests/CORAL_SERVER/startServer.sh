#!/bin/bash
usage="Usage: $0 (CoralAccess|CoolRegression|HLT) (stdout|fileServer|fileProxy0|fileProxy) (LCGCMT_xx|cwd)"
if [ "$3" == "" ] || [ "$4" != "" ]; then
  echo "$usage"; exit 1
fi

test=$1
cout=$2
lcg=$3
shift
shift
shift

pushd `dirname $0` > /dev/null
dir=`pwd`

. test_functions.sh

if [ "$test" == "CoralAccess" ]; then
  baselog=serverLog
elif [ "$test" == "CoolRegression" ]; then
  baselog=coolServerLog
elif [ "$test" == "HLT" ]; then
  baselog=hltServerLog
else
  echo "$usage"; exit 1
fi
basedir=`cd $dir/../..; pwd`/logs/CORAL_SERVER/$test

if [ "$cout" == "stdout" ]; then
  outfile=/dev/stdout
  csvfile=/tmp/$USER/${baselog}.csv
  echo "Server output to stdout:" $outfile
else
  if [ "$cout" == "fileServer" ]; then
    filename=${baselog}_server
  elif [ "$cout" == "fileProxy0" ]; then
    filename=${baselog}_proxy0
  elif [ "$cout" == "fileProxy" ]; then
    filename=${baselog}_proxy
  else
    echo "$usage"; exit 1
  fi
  outfile=${basedir}/server/${filename}.txt
  csvfile=${basedir}/server/${filename}.csv
  echo "Server output file:" $outfile
fi
echo "Server csv monitor:" $csvfile

if [ "$lcg" == "LCGCMT_${lcg/#LCGCMT_}" ]; then
  lcgconf=/afs/cern.ch/sw/lcg/app/releases/LCGCMT/$lcg/LCG_Configuration/cmt/requirements
  if [ ! -f "$lcgconf" ]; then
    echo "ERROR! $lcgconf does not exist"
    exit 1
  fi
  release=/afs/cern.ch/sw/lcg/app/releases/CORAL/`grep CORAL $lcgconf | awk '{print substr($3,2,length($3)-2)}'`
  lcgseries=${lcg:7:2}
  if [[ "$lcgseries" < 59 ]]; then
    echo "ERROR! Only LCGCMT releases >= 59 are supported"
    exit 1
  fi
elif [ "$lcg" == "cwd" ]; then
  release=`cd ../../..; pwd`
else
  echo "$usage"; exit 1
fi

date +'%a %b %d %H:%M:%S.%N %Z %Y' > $outfile
echo "Kill any running instance of coralServer and sleep 1s" >> $outfile
killall -9 coralServer >& /dev/null
sleep 1

# Reset CMTCONFIG for HLT tests
if [ "$test" == "HLT" ] && [ "$CMTCONFIG" != "i686-slc5-gcc43-dbg" ] && [ "$CMTCONFIG" != "i686-slc5-gcc43-opt" ]; then export CMTCONFIG=i686-slc5-gcc43-dbg; fi

# Setup CMT and environment from src/config/cmt
pushd $release/src/config/cmt > /dev/null
export SITEROOT=/afs/cern.ch # workaround for bug #87325 in older releases
echo "Setup CMT environment using CMT_env.sh" >> $outfile
if [ ! -f CMT_env.sh ]; then
  echo "ERROR! No CMT_env.sh in `pwd`" >> $outfile
  if [ "$cout" != "stdout" ]; then echo "ERROR! No CMT_env.sh in `pwd`"; fi
  exit 1
fi 
. CMT_env.sh >> $outfile
if [ ! -f setup.sh ]; then
  echo "ERROR! No setup.sh in `pwd`" >> $outfile
  if [ "$cout" != "stdout" ]; then echo "ERROR! No setup.sh in `pwd`"; fi
  exit 1
fi 
. setup.sh >> $outfile
popd > /dev/null

# Setup CORAL_AUTH_PATH and CORAL_DBLOOKUP_PATH
# [NB for HLT use the 'local' settings: the server connects via OracleAccess!]
setupDatabase $test local >> $outfile

unset CORALSERVER_ENABLECONNECTIONSHARING
unset CORALSERVER_DEBUG
unset CORALSTUBS_DEBUG
unset CORALSOCKETS_DEBUG
unset CORAL_MSGLEVEL
unset CORAL_MESSAGEREPORTER_FORMATTED
unset CORAL_MSGFORMAT
unset CORAL_ORA_SKIP_TRANS_READONLY

if [ "$test" != "HLT" ]; then
  ###export CORALSERVER_ENABLECONNECTIONSHARING=1
  export CORALSERVER_DEBUG=1
  ###export CORALSTUBS_DEBUG=1
  export CORALSOCKETS_DEBUG=1
fi

###export CORAL_MSGLEVEL=error
###export CORAL_MSGLEVEL=verbose

unset FRONTIER_SERVER # workaround for bug #87307 (not really needed here)

###msglevel=""      # error (default)
msglevel="-vv"      # info
###msglevel="-vvvv" # verbose

###export CORAL_MESSAGEREPORTER_FORMATTED=1
###export CORAL_MSGFORMAT=COOL
###export CORAL_MSGFORMAT=ATLAS
###msgformat=""
msgformat="-o ATLAS"

###export CORAL_ORA_SKIP_TRANS_READONLY=1
###rotrans=""
rotrans="-y ON"
###rotrans="-y OFF"

###monitoring=""
monitoring="-e $csvfile"

date +'%a %b %d %H:%M:%S.%N %Z %Y' >> $outfile
pushd $release/src/config/cmt > /dev/null
echo "CMTCONFIG is $CMTCONFIG" >> $outfile
echo "Using LCG:    "`cmt show macro_value LCG_config_version` >> $outfile
echo "Using LCGCMT: "`cmt show macro_value LCGCMT_home` >> $outfile
echo "Using Boost:  "`cmt show macro_value Boost_home` >> $outfile
echo "Using CORAL:  "`cmt show macro_value CORAL_home` >> $outfile
echo "Using ROOT:   "`cmt show macro_value ROOT_home` >> $outfile
echo "Using COOL:   "`cmt show macro_value COOL_home` >> $outfile
popd > /dev/null
echo "*******************************************************************" >> $outfile
# Kill using SIGTERM and wait 2s to ensure dump of monitor.csv file at the end
/usr/bin/time -ao/dev/stdout -f"=== TIMER (SERVER) %Uuser %Ssystem %eelapsed %PCPU" coralServer -p 40007 $msglevel $msgformat $rotrans $monitoring >> $outfile 
echo "*******************************************************************" >> $outfile 
date +'%a %b %d %H:%M:%S.%N %Z %Y' >> $outfile

### To debug the server enable the following two lines
### (you must have up the environment for the appropriate release first!)
###pid=`ps -C coralServer -o pid=`
###gdb coralServer $pid
###(gdb) info threads
###(gdb) thread apply all where

if [ "$cout" != "stdout" ]; then
  \mv ${outfile} ${outfile}.old
  ./sedDate.sh ${outfile}.old > ${outfile}
  ###\rm ${outfile}.old
fi

# Transpose the server csv file
if [ -f "$csvfile" ]; then
  ./csvTranspose.sh $csvfile
  \rm -f ${csvfile}.bak
fi

popd > /dev/null
