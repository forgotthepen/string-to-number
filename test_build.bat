@echo off
pushd "%~dp0"
setlocal

call :build_cxx 23 || exit /b 1
call :build_cxx 20 || exit /b 1
call :build_cxx 17 || exit /b 1
call :build_cxx 14 || exit /b 1

exit /b 0


:build_cxx
rmdir /s /q build-win/cxx_%~1
cmake -S . -B build-win/cxx_%~1 -DCMAKE_CXX_STANDARD=%~1 || exit /b 1
cmake --build build-win/cxx_%~1 --clean-first -v -j || exit /b 1
exit /b 0
