@echo off

rem Examples:
rem cmtBroadcast make                    -> lib, utilitlies
rem cmtBroadcast make all                -> lib, utilities
rem cmtBroadcast make tests              -> tests
rem cmtBroadcast make examples           -> examples
rem cmtBroadcast make all tests          -> lib, utilities, tests
rem cmtBroadcast make all tests examples -> lib, utilities, tests, examples

rem See http://www.microsoft.com/resources/documentation/windows/xp/all/proddocs/en-us/ntcmds_shelloverview.mspx?mfr=true
setlocal
set CMTPATH=
set CMTUSERCONTEXT=
if "%1"=="" (
  cmt broadcast "set CMTUSERCONTEXT=%~d0%~p0USERCONTEXT\%USERNAME%"
) else (
  cmt broadcast "set CMTUSERCONTEXT=%~d0%~p0USERCONTEXT\%USERNAME%&& %*"
)
endlocal
