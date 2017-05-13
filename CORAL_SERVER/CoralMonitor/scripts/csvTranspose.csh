#! /bin/tcsh -f

if ( "$1" == "-c" ) then
  set cat=1
  shift
else
  set cat=0
endif

if ( "$1" == "-h" ) then
  set header="-v noHeader=1"
  shift
else
  set header="-v noHeader=0"
endif

if ( "$1" == "-s" ) then
  set strict="-v strict=1"
  shift
else
  set strict="-v strict=0"
endif

if ( "$1" == "-v" ) then
  set verb="-v verb=1"
  shift
else
  set verb="-v verb=0"
endif

if ( "$1" == "" || "$2" != "" ) then
  echo "Usage: $0 [-c] [-h] [-s] [-v] file.csv"
  echo "Options: -c cat (cat transposed csv at the end)"
  echo "Options: -h skip header (do not repeat the header in the last column)"
  echo "Options: -s strict mode (require constant number of columns)"
  echo "Options: -v verbose mode"
  exit 1
endif

set theFileCsv=$1
set theDir=`dirname $theFileCsv`
set theFileCsv=`basename $theFileCsv`
set theFile=`basename $theFileCsv .csv`
if ( "${theFileCsv}" != "${theFile}.csv" ) then
  echo "ERROR! File name does not end by .csv"
  exit 1
endif

set theAwkDir=`dirname $0`
set theAwkDir=`cd $theAwkDir; pwd`

echo "Original   csv file: ${theDir}/${theFile}.csv"
echo "Transposed csv file: ${theDir}/${theFile}-transpose.csv"
if ( ! -f ${theDir}/${theFile}.csv ) then
  echo "WARNING! File ${theDir}/${theFile}.csv does not exist..."
else
  awk $strict $header $verb -f $theAwkDir/csvTranspose.awk ${theDir}/${theFile}.csv > ${theDir}/${theFile}-transpose.csv
  if ( $cat == 1 ) cat ${theFile}-transpose.csv
endif

