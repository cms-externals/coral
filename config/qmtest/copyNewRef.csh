#! /bin/tcsh -f

#echo "copyNewRef.csh - You must comment out this line!"; exit 1

set theScrDir=`dirname $0`
set theScrDir=`cd $theScrDir; pwd`

pushd $theScrDir > /dev/null
set theLogs=`ls *.log`
foreach theLog ( $theLogs )
  set theTest=`basename $theLog .log`
  echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  echo ${theTest}
  \rm -rf ${theTest}.log.oval
  \rm -rf ${theTest}.ref.oval
  cat ${theTest}.log | grep -a '\[OVAL\]' > ${theTest}.log.oval
  cat ${theTest}.ref | grep -a '\[OVAL\]' > ${theTest}.ref.oval
  echo diff ${theTest}.log.oval ${theTest}.ref.oval
  diff ${theTest}.log.oval ${theTest}.ref.oval
  set theRef=`ls ${theScrDir}/../../Tests/*/${theTest}.ref`
  echo cp ${theTest}.log.oval ${theRef}
  cp ${theTest}.log.oval ${theRef}
end 
popd > /dev/null
