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
include_directories(${CMAKE_SOURCE_DIR}/net)
set(base_LIB
base/alloc.c
base/base_time.cc
base/corvus.c
base/logging.c
base/mbuf.c
base/my_thread.cc
base/platform_linux.c
)
add_library(base STATIC ${base_LIB})

set(net_LIB
net/ae.c
net/ae_epoll.c
net/anet.c
)
add_library(net STATIC ${net_LIB})

set(EXECUTABLE_NAME "server")
add_executable(${EXECUTABLE_NAME} server.cc)
target_link_libraries(${EXECUTABLE_NAME}  base pthread)
set(EXECUTABLE_NAME "client")
add_executable(${EXECUTABLE_NAME} client.cc )
target_link_libraries(${EXECUTABLE_NAME}  base pthread)

set(EXECUTABLE_NAME "mbuf")
add_executable(${EXECUTABLE_NAME} base/mbuf_test.c )
target_link_libraries(${EXECUTABLE_NAME}  base pthread)

set(EXECUTABLE_NAME "t_time")
add_executable(${EXECUTABLE_NAME} time_test.cc)
target_link_libraries(${EXECUTABLE_NAME}  base pthread)


set(EXECUTABLE_NAME "t_bbr_client")
add_executable(${EXECUTABLE_NAME} tcp_bbr_client.cc)
target_link_libraries(${EXECUTABLE_NAME}  net base pthread)

set(EXECUTABLE_NAME "t_bbr_server")
add_executable(${EXECUTABLE_NAME} tcp_bbr_server.cc)
target_link_libraries(${EXECUTABLE_NAME}  net base pthread)
