for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" /version [17.0^,18^) /property installationPath`) do (
  set VSDIR=%%i
  echo %%i
)
set VSCMD_DEBUG=True
call "%VSDIR%\Common7\Tools\VsDevCmd.bat"
cd %~dp0
