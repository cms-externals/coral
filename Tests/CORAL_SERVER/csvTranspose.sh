#! /bin/bash

if [ "$1" == "-c" ]; then
  cat=1
  shift
else
  cat=0
fi

if [ "$1" == "-h" ]; then
  header="-v noHeader=1"
  shift
else
  header="-v noHeader=0"
fi

if [ "$1" == "-s" ]; then
  strict="-v strict=1"
  shift
else
  strict="-v strict=0"
fi

if [ "$1" == "-v" ]; then
  verb="-v verb=1"
  shift
else
  verb="-v verb=0"
fi

if [ "$1" == "" ] || [ "$2" != "" ]; then
  echo "Usage: $0 [-c] [-h] [-s] [-v] file.csv"
  echo "Options: -c cat (cat transposed csv at the end)"
  echo "Options: -h skip header (do not repeat the header in the last column)"
  echo "Options: -s strict mode (require constant number of columns)"
  echo "Options: -v verbose mode"
  exit 1
fi

theFileCsv=$1
theDir=`dirname $theFileCsv`
theFileCsv=`basename $theFileCsv`
theFile=`basename $theFileCsv .csv`
if [ "${theFileCsv}" != "${theFile}.csv" ]; then
  echo "ERROR! File name does not end by .csv"
  exit 1
fi

theAwkDir=`dirname $0`
theAwkDir=`cd $theAwkDir; pwd`

if [ ! -f ${theDir}/${theFile}.csv ]; then
  echo "WARNING! File ${theDir}/${theFile}.csv does not exist..."
else
  awk $strict $header $verb -f $theAwkDir/csvTranspose.awk ${theDir}/${theFile}.csv > ${theDir}/${theFile}-transpose.csv
  if [ $cat == 1 ]; then cat ${theFile}-transpose.csv; fi
  \mv ${theDir}/${theFile}.csv ${theDir}/${theFile}-original.csv
fi

echo "Original   csv file: ${theDir}/${theFile}-original.csv"
echo "Transposed csv file: ${theDir}/${theFile}-transpose.csv"

