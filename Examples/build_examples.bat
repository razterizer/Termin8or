call ..\..\Core\build.bat

mkdir x86\Release

cl examples.cpp /I..\.. /I..\include\Termin8or /std:c++20 /Fex86\Release\examples.exe /Fox86\Release\examples.obj

if %errorlevel% neq 0 (
    echo Build failed with error code %errorlevel%.
    exit /b %errorlevel%
)

echo Build succeeded.
