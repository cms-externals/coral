#! /bin/tcsh -f

echo "--- Disk usage excluding CVS ---"
du  --exclude='*CVS*' */*

echo "--- Disk usage excluding CVS, .old and .csv files ---"
du  --exclude='*CVS*' --exclude='*.old' --exclude='*.csv' */*

###echo "--- Detailed disk usage excluding CVS, .old and .csv files ---"
###du  --exclude='*CVS*' --exclude='*.old' --exclude='*.csv' */*/*
