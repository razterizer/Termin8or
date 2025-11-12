REM call ..\..\Core\build.bat
REM
REM mkdir x86\Release
REM
REM cl examples.cpp /I..\.. /I..\include\Termin8or /std:c++20 /Fex86\Release\examples.exe /Fox86\Release\examples.obj
REM
REM if %errorlevel% neq 0 (
REM     echo Build failed with error code %errorlevel%.
REM     exit /b %errorlevel%
REM )
REM
REM echo Build succeeded.

call ..\..\Core\build.bat

cd examples.vs

SET configuration="Release"
IF "%~1" == "Debug" SET configuration="Debug"
SET target="x64"
IF "%~2" == "x86" SET target="x86"
msbuild examples.vs.sln /p:Configuration=%configuration% /p:Platform=%target%

cd ..

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
