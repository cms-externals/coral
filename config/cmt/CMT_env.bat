@echo off
rem $Id: CMT_env.bat,v 1.7 2011/01/21 11:29:51 avalassi Exp $

rem Set CMTCONFIG
set CMTCONFIG=i686-winxp-vc9-dbg

rem Set AFS, SITEROOT, CMTROOT
set AFS=Z:\
set SITEROOT=%AFS%cern.ch
set CMTROOT=%SITEROOT%\sw\contrib\CMT\v1r20p20090520
rem set CMTROOT=%SITEROOT%\sw\contrib\CMT\v1r21

rem Set AFSHOME
rem See http://groups.google.com/group/alt.msdos.batch.nt/msg/923423bf3143fcdd
rem See http://www.student.northpark.edu/pemente/sed/bat_env.htm
if not defined AFSHOME (
  set AFSHOME=%AFS%cern.ch\user\%USERNAME:~0,1%\%USERNAME%
)

rem Setup Visual Studio 9.0
call "%VS90COMNTOOLS%\vsvars32.bat" > NUL

rem Setup CMT
rem call %CMTROOT%\mgr\setup.bat
set CMTBIN=VisualC
if not defined CMTCONFIG set CMTCONFIG=%CMTBIN%
set PATH=%CMTROOT%\%CMTBIN%;%PATH%

rem Set VERBOSE (verbose build for CMT v1r20p2008xxxx or higher)
set VERBOSE=1

rem Set CMTPROJECTPATH and unset CMTPATH
set CMTPATH=
set CMTPROJECTPATH=%SITEROOT%\sw\lcg\app\releases

rem Set CMTUSERCONTEXT
rem See http://www.microsoft.com/resources/documentation/windows/xp/all/proddocs/en-us/ntcmds_shelloverview.mspx?mfr=true
rem See http://www.microsoft.com/resources/documentation/windows/xp/all/proddocs/en-us/percent.mspx?mfr=true
set CMTUSERCONTEXT=%~d0%~p0USERCONTEXT\%USERNAME%

rem Unset CMTSITE
set CMTSITE=

rem User-specific customizations
if exist %CMTUSERCONTEXT%\CMT_userenv.bat (
  echo [Customize user environment using %CMTUSERCONTEXT%\CMT_userenv.bat]
  call %CMTUSERCONTEXT%\CMT_userenv.bat
)

rem Printout AFS environment variables
echo AFS set to '%AFS%'
echo AFSHOME set to '%AFSHOME%'

rem Printout CMT environment variables
echo CMTCONFIG set to '%CMTCONFIG%'
echo CMTROOT set to '%CMTROOT%'
echo CMTPATH set to '%CMTPATH%'
echo CMTPROJECTPATH set to '%CMTPROJECTPATH%'
echo CMTUSERCONTEXT set to '%CMTUSERCONTEXT%'
if defined CMTINSTALLAREA (
  echo CMTINSTALLAREA set to '%CMTINSTALLAREA%'
) else (
  echo CMTINSTALLAREA is not set
)
if defined CMTSITE (
  echo CMTSITE set to '%CMTSITE%'
) else (
  echo CMTSITE is not set
)

