#! /bin/bash
test=`dirname $0`
test=`cd $test; pwd`
test=`basename $test`
###echo "$0 $*"
###echo "../allTests.sh $test $*"
../allTests.sh $test $*
if [ "$?" == 123 ]; then echo "Usage: $0 "`../allTests.sh 2>&1 | awk '{print $4, $5, $6}'`; fi
