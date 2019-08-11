PROJECT(project)
cmake_minimum_required(VERSION 2.6)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")
set(CMAKE_CXX_FLAGS "-fPIC")  
set(CMAKE_C_FLAGS "-fPIC") 
add_definitions(-D__STDC_FORMAT_MACROS)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/base)

set(base_LIB
base/alloc.c
base/corvus.c
base/logging.c
base/mbuf.c
base/platform_linux.c

)
add_library(base STATIC ${base_LIB})



set(EXECUTABLE_NAME "server")
add_executable(${EXECUTABLE_NAME} server.cc)
target_link_libraries(${EXECUTABLE_NAME}  base pthread)
set(EXECUTABLE_NAME "client")
add_executable(${EXECUTABLE_NAME} client.cc )
target_link_libraries(${EXECUTABLE_NAME}  base pthread)
