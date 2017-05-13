#! /bin/tcsh -f

#echo "cleanAll.csh - You must comment out this line!"; exit 1

set theScrDir=`dirname $0`
set theScrDir=`cd $theScrDir; pwd`

pushd $theScrDir > /dev/null
\rm -rf *.oval *.ref *.log
\rm -rf *.pyc *.txt *.db *.qmr *.qms 
\rm -rf log
popd > /dev/null
