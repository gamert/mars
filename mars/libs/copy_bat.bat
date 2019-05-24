
set _PROJ_=G:\xProject_dp\SDK\mars\mars


set _abi_=armeabi-v7a
@rem 首先编译
cd %_PROJ_%

c:/python27/python build_android.py %_abi_% %_abi_%
pause

copy %_PROJ_%\cmake_build\Android\comm\libcomm.a %_PROJ_%\libs\comm\prebuilt\android\%_abi_%
copy %_PROJ_%\cmake_build\Android\boost\libmars-boost.a %_PROJ_%\libs\boost\prebuilt\android\%_abi_%
copy %_PROJ_%\cmake_build\Android\xlog\libxlog.a %_PROJ_%\libs\xlog\prebuilt\android\%_abi_%

pause

set _abi_=x86
c:/python27/python build_android.py %_abi_% %_abi_%
pause
copy %_PROJ_%\cmake_build\Android\comm\libcomm.a %_PROJ_%\libs\comm\prebuilt\android\%_abi_%
copy %_PROJ_%\cmake_build\Android\boost\libmars-boost.a %_PROJ_%\libs\boost\prebuilt\android\%_abi_%
copy %_PROJ_%\cmake_build\Android\xlog\libxlog.a %_PROJ_%\libs\xlog\prebuilt\android\%_abi_%
pause