@echo off
@chcp 65001>nul

if "%VSCMD_VER%" == "" (
    @REM Replace the path with your own "VsDevCmd.bat" path.
    call "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/Tools/VsDevCmd.bat"
)

msbuild ImmPad.vcxproj -property:Configuration=Debug;Platform=x64

pause
