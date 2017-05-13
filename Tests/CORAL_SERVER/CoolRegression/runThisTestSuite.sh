#!/bin/bash
test=`dirname $0`
test=`cd $test; pwd`
test=`basename $test`
###echo "$0 $*"
###echo "../runTestSuite.sh $test $*"
../runTestSuite.sh $test $*
if [ "$?" == 123 ]; then echo "Usage: $0 "`../runTestSuite.sh 2>&1 | awk '{print $4, $5}'`; fi

