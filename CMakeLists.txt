cmake_minimum_required(VERSION 3.5)

project(TOY_Interpreter)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

if(UNIX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17 -no-pie")

  # DEBUG AND RELEASE SYSTEM
  if(${CMAKE_BUILD_TYPE} MATCHES Debug)

    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/Linux/Debug)

  elseif(${CMAKE_BUILD_TYPE} MATCHES Release)

    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/Linux/Release)

  endif()
  # END OF DEBUG AND RELEASE SYSTEM

elseif(WIN32)

  message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  
  # DEBUG AND RELEASE SYSTEM
  if(${CMAKE_BUILD_TYPE} MATCHES Debug)

    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/Windows/Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-subsystem,windows -std=c++17")
  
  elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
  
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/Windows/Release)
  
  endif()
  # END OF DEBUG AND RELEASE SYSTEM
  
  
endif()
  
  # SETS THE EXECUTABLE PATH WHEN COMPILING
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
  
  
file(GLOB source_files "${CMAKE_SOURCE_DIR}/src/*.cpp")
  
#include_directories(${CMAKE_SOURCE_DIR}/src/vendors)
include_directories(${CMAKE_SOURCE_DIR}/src/headers)
  
add_executable(${PROJECT_NAME} ${source_files})
  
