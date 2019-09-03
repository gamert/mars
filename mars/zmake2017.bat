@rem mkdir proj.ios
@rem cd proj.ios
@rem cmake -G "Xcode" ..

cd %~dp0
mkdir cmake_build_x64 
cd cmake_build_x64

cmake -G "Visual Studio 15 2017 Win64" ..
pause