cmake_minimum_required(VERSION 2.6.2)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")
set(CMAKE_FIND_FRAMEWORK NEVER FORCE)
set(CMAKE_FIND_APPBUNDLE NEVER)

project(DCOURIER)
# The Version Number
set(DCOURIER_VERSION_MAJOR 0)
set(DCOURIER_VERSION_MINOR 2)
set(DCOURIER_VERSION_PATCH 1)
set(DCOURIER_VERSION_BUILD 1)
set(DCOURIER_VERSION_RC "")

configure_file(
	"${PROJECT_SOURCE_DIR}/src/DCOURIER_Config.h.in"
	"${PROJECT_SOURCE_DIR}/src/DCOURIER_Config.h"
)

include_directories("${PROJECT_SOURCE_DIR}/src/")

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -L./lib/ -Wl,--no-as-needed -pthread -m64 -lstdc++")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/exe/)
# set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")    # Add debug information

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    execute_process(COMMAND uname OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)
    set(LINUX_LIBRARIES pthread)
endif(CMAKE_SYSTEM_NAME MATCHES Linux)

set(DCOURIER_src
	src/main.cpp
	src/out_file.hpp
	src/utils.hpp
	src/Com.hpp
	src/Exception.h
	src/fwlib32.h
)
file(GLOB adapter_headers *.hpp)

#add_library(fwlib32 SHARED IMPORTED)
#set_property(TARGET fwlib32 PROPERTY IMPORTED_LOCATION "../lib/libfwlib32.so.1.0.0")

add_executable(DCOURIER ${DCOURIER_src})
target_link_libraries(DCOURIER ${LINUX_LIBRARIES})

install(TARGETS DCOURIER RUNTIME DESTINATION bin)

