#!/bin/bash

pushd `dirname $0` > /dev/null
dir=`pwd`
if [ ! -e test_functions.sh ]; then
  echo "ERROR! Please include test_functions.sh in its own directory"
  exit 1
fi
popd > /dev/null

function setupDatabase() 
{
  usage="ERROR! Usage (function): $FUNCNAME (CoralAccess|CoolRegression|HLT) (local|Fac|StbFac|server|proxy0|proxy|frontier|squid)"
  if [ "$2" == "" ] || [ "$3" != "" ]; then
    echo "$usage"; exit 1
  fi
  test=$1
  conn=$2
  shift
  shift
  # Setup CORAL_AUTH_PATH, CORAL_DBLOOKUP_PATH (and COOLTESTDBO_xx if relevant)
  # [NB this should be done after ATLAS_CMT_env that may change these settings]
  if [ $test == "CoralAccess" ]; then
    export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db
    export CORAL_DBLOOKUP_PATH=/afs/cern.ch/sw/lcg/app/pool/db
    # No real need for a different setup for user avalassi 
    # Keep the same setup as for user lcgnight (test fix for bug #87175)
    ###if [ "$USER" == "avalassi" ]; then
    ###  export CORAL_AUTH_PATH=${HOME}/private
    ###  export CORAL_DBLOOKUP_PATH=${HOME}/private
    ###fi
  elif [ $test == "CoolRegression" ]; then
    export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db
    export CORAL_DBLOOKUP_PATH=/afs/cern.ch/sw/lcg/app/pool/db
    export COOLTESTDBO_CS0="COOL-Oracle-lcgnight/COOLTEST"
    # No real need for a different setup for user avalassi 
    # Keep the same setup as for user lcgnight (test fix for bug #87175)
    ###if [ "$USER" == "avalassi" ]; then
    ###  export CORAL_AUTH_PATH=${HOME}/private
    ###  export CORAL_DBLOOKUP_PATH=${HOME}/private
    ###  export COOLTESTDBO_CS0="COOL-Oracle-${USER}/COOLTEST"
    ###fi
    export COOLTESTDBO_CS1="coral_TEST://SFac&$COOLTESTDBO_CS0"
    export COOLTESTDBO_CS2="coral_TEST://CStb+SStb+SFac&$COOLTESTDBO_CS0"
    export COOLTESTDBO_CS3="coral://localhost:40007&$COOLTESTDBO_CS0"
    export COOLTESTDBO_CS4="coral://localhost:40008&$COOLTESTDBO_CS0"
    if [ "$conn" == "local" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS0
    elif [ "$conn" == "Fac" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS1
    elif [ "$conn" == "StbFac" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS2
    elif [ "$conn" == "server" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS3
    elif [ "$conn" == "proxy0" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS4
    elif [ "$conn" == "proxy" ]; then
      export COOLTESTDB=$COOLTESTDBO_CS4
    else
      echo "PANIC! Test $test is not supported for connection mode $conn"
      exit 1
    fi
    echo "COOLTESTDB: "$COOLTESTDB
  elif [ $test == "HLT" ]; then
    if [ "$conn" == "local" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/local
      export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db/athenaMT
    elif [ "$conn" == "Fac" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/Fac
      export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db/athenaMT
    elif [ "$conn" == "StbFac" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/StbFac
      export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db/athenaMT
    elif [ "$conn" == "server" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/server
      export CORAL_AUTH_PATH=
    elif [ "$conn" == "proxy0" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/proxy # Same as proxy
      export CORAL_AUTH_PATH=
    elif [ "$conn" == "proxy" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/proxy
      export CORAL_AUTH_PATH=
    elif [ "$conn" == "frontier" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/frontier
      export CORAL_AUTH_PATH=
      ###export CORAL_AUTH_PATH=/afs/cern.ch/sw/lcg/app/pool/db/athenaMT # TEMPORARY (debug bug #69901)
    elif [ "$conn" == "squid" ]; then
      export CORAL_DBLOOKUP_PATH=$dir/$test/dblookup/squid
      export CORAL_AUTH_PATH=
    else
      echo "$usage"; exit 1
    fi
    echo CORAL_DBLOOKUP_PATH=$CORAL_DBLOOKUP_PATH
    echo CORAL_AUTH_PATH=$CORAL_AUTH_PATH
  fi
}
