#! /bin/tcsh -f

###echo "summarizeAll.csh - You must comment out this line!"; exit 1

# Check arguments - define the platforms to analyze
set thePlatformsSl5="i686-slc5-gcc43-dbg\
                     i686-slc5-gcc43-opt\
                     i686-slc5-icc11-dbg\
                     i686-slc5-icc11-opt\
                     x86_64-slc5-gcc43-dbg\
                     x86_64-slc5-gcc43-opt\
                     x86_64-slc5-icc11-dbg\
                     x86_64-slc5-icc11-opt"
set thePlatformsOsx="i386-mac106-gcc42-dbg\
		     i386-mac106-gcc42-opt\
		     x86_64-mac106-gcc42-dbg\
		     x86_64-mac106-gcc42-opt"
set thePlatformsWin="i686-winxp-vc9-dbg"
if ( "$2" != "" || "$1" == "" ) then
  echo "Usage: `basename ${0}` all|sl5|osx|win|<CMTCONFIGs>"
  exit 1
else if ( "$1" == "all" ) then
  set thePlatforms=`echo $thePlatformsSl5 $thePlatformsOsx $thePlatformsWin`
else if ( "$1" == "sl5" ) then
  set thePlatforms=`echo $thePlatformsSl5`
else if ( "$1" == "osx" ) then
  set thePlatforms=`echo $thePlatformsOsx`
else if ( "$1" == "win" ) then
  set thePlatforms=`echo $thePlatformsWin`
else
  set thePlatforms="$*"
endif

# Remove the argument - otherwise it confuses CMT setup.csh
set argv[1]=""

# Locate the config/qmtest directory
set configQmtest=`dirname ${0}`
set configQmtest=`cd $configQmtest/../../config/qmtest; pwd`

# Go to the cmt directory and setup cmt
#pushd ../cmt >& /dev/null
#source CMT_env.csh >& /dev/null
#source setup.csh
#popd >& /dev/null

# Go to the qmtest directory and produce the test summaries
cd $configQmtest
foreach thePlatform ( ${thePlatforms} )
  if ( "${thePlatform}" != "" ) then
    set theQmr=../../logs/qmtest/${thePlatform}.qmr
    set theXml=../../logs/qmtest/${thePlatform}.xml
    set theSum=../../logs/qmtest/${thePlatform}.summary
    echo "Produce qmtest XML report from ${theQmr}"
    if ( -f ${theQmr} ) then
      \rm -rf ${theXml}
      ### *** NB It is also possible to qmtest report from multiple xxx.qmr ***
      ###qmtest report -o ${theXml} ${theQmr} # Fix bug #32789
      ../../logs/qmtest/myQmtest230 report -o ${theXml} ${theQmr}
      echo "Remove DOCTYPE from the XML report"
      cat ${theXml} | grep -v '<\!DOCTYPE' > ${theXml}.new
      \mv ${theXml}.new ${theXml}
      echo "Remove 'Invalid folder node path' from the XML report"
      cat ${theXml} | grep -v 'Invalid folder node path' > ${theXml}.new
      \mv ${theXml}.new ${theXml}
      echo "Remove binary characters from the XML report"
      python ../../logs/qmtest/removeBinaryChars.py ${theXml} ${theXml}.new
      \mv ${theXml}.new ${theXml}
    else
      echo "WARNING! File not found: ${theQmr}"
    endif
    echo "Produce qmtest summary from ${theXml}"
    if ( -f ${theXml} ) then
      \rm -rf ${theSum}
      python ../../logs/qmtest/parseQmtestRun.py ${theXml} > ${theSum}
      ###setenv QMTEST_DISABLE_PATCH_TASK18784 1
      ###setenv QMTEST_DISABLE_PATCH_TASK21088 1
      setenv QMTEST_DISABLE_PATCH_TASK21216 1
      ###qmtest summarize --format batch ${theXml} >> ${theSum}
      qmtest summarize --format brief ${theXml} >> ${theSum}
      ###qmtest summarize --format full ${theXml} >> ${theSum}
      cat ${theSum} | grep -v '^Node <DOM Element: result' > ${theSum}.new
      \mv ${theSum}.new ${theSum}
      ###cat ${theSum} | sed "s/CMTCONFIG = ${CMTCONFIG}/CMTCONFIG = ${thePlatform}/" > ${theSum}.new
      ###\mv ${theSum}.new ${theSum}
    else
      echo "ERROR! File not found: ${theXml}"
    endif
    echo "Done!"
  endif
end

