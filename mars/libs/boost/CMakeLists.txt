cmake_minimum_required(VERSION 3.6)

set(lib_name boost)
set(target_name mars_${lib_name})
project(${lib_name})

message("target_name = ${target_name}")

include(../Config.cmake)

add_library(${target_name} STATIC IMPORTED GLOBAL)

set_target_properties(${target_name} PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
if(WINDOWS)
  set_target_properties(${target_name} PROPERTIES
    CC_DEPEND_DLLS "${platform_spec_path}/lua51.dll"
    IMPORTED_LOCATION "${platform_spec_path}/lua51.lib"
  )
else()
  set(platform_spec_path ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/${ANDROID_ABI}/libmars-${lib_name}.a)
  message("platform_spec_path = ${platform_spec_path}")
  set_target_properties(${target_name} PROPERTIES
    IMPORTED_LOCATION "${platform_spec_path}"
  )
endif()
