@echo off
setlocal

cmake -S . -B build -G Ninja && cmake --build build && pushd build && PackageMan.exe %* & popd
