#!/bin/bash
test="HLT"
usage="Usage: $0 (CoralAccess|CoolRegression|HLT) (stdout|fileProxy0|fileProxy) (LCGCMT_xx|cwd)"
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

if [ "$test" == "CoralAccess" ]; then
  baselog=proxyLog
elif [ "$test" == "CoolRegression" ]; then
  baselog=coolProxyLog
elif [ "$test" == "HLT" ]; then
  baselog=hltProxyLog
else
  echo "$usage"; exit 1
fi
basedir=`cd $dir/../..; pwd`/logs/CORAL_SERVER/$test

if [ "$cout" == "stdout" ]; then
  outfile=/dev/stdout
  echo "Proxy  output to stdout:" $outfile
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
  outfile=${basedir}/proxy/${filename}.txt
  echo "Proxy  output file:" $outfile
fi

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
echo "Kill any running instance of coralServerProxy and sleep 1s" >> $outfile
killall -9 coralServerProxy >& /dev/null
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

unset CORAL_AUTH_PATH
unset CORAL_DBLOOKUP_PATH

unset CORALSERVER_DEBUG
unset CORALSTUBS_DEBUG
unset CORALSOCKETS_DEBUG
#export CORALSERVER_DEBUG=1
#export CORALSTUBS_DEBUG=1
#export CORALSOCKETS_DEBUG=1

msglevel=""       # info (default)
###msglevel="-vv" # verbose

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
/usr/bin/time -ao/dev/stdout -f"=== TIMER (SPROXY) %Uuser %Ssystem %eelapsed %PCPU" coralServerProxy -m -p 40008 -s 40007 0.0.0.0 $msglevel >> $outfile 
echo "*******************************************************************" >> $outfile 
date +'%a %b %d %H:%M:%S.%N %Z %Y' >> $outfile

if [ "$cout" != "stdout" ]; then
  # Remove CVS reserved words from the logfiles
  \mv ${outfile} ${outfile}.old
  cat ${outfile}.old | sed 's/\$''Name/$CvsName/' | sed 's/\$''Id/$CvsId/' > ${outfile}
  \rm ${outfile}.old
fi

if [ "$cout" != "stdout" ]; then
  \mv ${outfile} ${outfile}.old
  ./sedDate.sh ${outfile}.old > ${outfile}
  ###\rm ${outfile}.old
fi

popd > /dev/null
