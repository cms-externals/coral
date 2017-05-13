#!/bin/bash
usage="Usage: $0 (CoralAccess|CoolRegression|HLT) (local|Fac|StbFac|server|proxy0|proxy|frontier|squid) (stdout|file) (LCGCMT_xx|cwd)"
if [ "$4" == "" ] || [ "$5" !=  "" ]; then
  echo "$usage"; exit 1
fi

echo "-----------------------------------------------------------------------"
echo $0 $1 $2 $3 $4
test=$1
conn=$2
cout1=$3
cout2=$3
lcg=$4
shift
shift
shift
shift

if [ $test != "CoralAccess" ] && [ $test != "CoolRegression" ] && [ $test != "HLT" ]; then
  echo "$usage"; exit 1
fi

if [ "$conn" == "local" ] || [ "$conn" == "Fac" ] || [ "$conn" == "StbFac" ] || [ "$conn" == "frontier" ] || [ "$conn" == "squid" ]; then
  startServer=0
  startProxy=0
elif [ "$conn" == "server" ]; then
  startServer=1
  startProxy=0
  if [ "$cout2" == "file" ]; then cout2="fileServer"; fi
elif [ "$conn" == "proxy0" ]; then
  startServer=1
  startProxy=1
  if [ "$cout2" == "file" ]; then cout2="fileProxy0"; fi
elif [ "$conn" == "proxy" ]; then
  startServer=1
  startProxy=1
  if [ "$cout2" == "file" ]; then cout2="fileProxy"; fi
else
  echo "$usage"; exit 1
fi

if [ $cout1 != "stdout" ] && [ $cout1 != "file" ]; then
  echo "$usage"; exit 1
fi

if [ "$lcg" == "LCGCMT_${lcg/#LCGCMT_}" ]; then
  echo "Testing CORAL from installed release $lcg"
  if [ $test != "HLT" ]; then
    echo "SORRY! Test $test is only supported for 'cwd' for the moment..."
    echo "$usage"; exit 1
  fi
  lcgconf=/afs/cern.ch/sw/lcg/app/releases/LCGCMT/$lcg/LCG_Configuration/cmt/requirements
  if [ ! -f $lcgconf ]; then
    echo "ERROR! $lcgconf does not exist"
    exit 1
  fi
  lcgseries=${lcg:7:2}
  if [[ $lcgseries < 59 ]]; then
    echo "ERROR! Only LCGCMT releases >=59 are supported"
    exit 1
  fi
elif [ "$lcg" == "cwd" ]; then
  echo "Testing CORAL from current build (NOT from an installed release)"
else
  echo "$usage"; exit 1
fi

pushd `dirname $0` > /dev/null
dir=`pwd`

if [ $startServer == 1 ]; then 
  ./startServer.sh $test $cout2 $lcg &
fi
if [ $startProxy == 1 ]; then
  ./startProxy.sh $test $cout2 $lcg &
fi
sleep 1s # Allow time to kill all running coralServer and coralServerProxy
./allTests.sh $test $conn $cout1 $lcg
echo "Client: returned control"

if [ $startProxy == 1 ]; then
  echo "Proxy : kill it using SIGTERM"
  killall coralServerProxy
fi

if [ $startServer == 1 ]; then
  echo "Server: kill it using SIGTERM"
  killall coralServer # Send SIGTERM: server will cleanly shutdown within ~2s
  echo "Server: sleep 3s to let it cleanly shutdown"
  sleep 3 # Let coralServer cleanly shutdown to complete its dump to csv file
  echo "Server: slept 3s, it should be gone by now"
fi
popd > /dev/null

