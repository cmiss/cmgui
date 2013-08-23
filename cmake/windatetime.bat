@echo off
setlocal
for /f "usebackq tokens=1,2 delims==" %%i in (`wmic os get LocalDateTime /VALUE 2^>nul`) do (
  if '.%%i.'=='.LocalDateTime.' (
    set ldt=%%j
    goto format
  )
)
:format
set myformat=%ldt:~6,2%/%ldt:~4,2%/%ldt:~0,4% %ldt:~8,2%:%ldt:~10,2%:%ldt:~12,2%
echo %myformat%
endlocal