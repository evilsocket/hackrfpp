cmake_minimum_required(VERSION 2.8.4)
project(hackrfpp CXX C)

set( PROJ_SOURCES src/main.cpp )

add_executable(hackrfpp ${PROJ_SOURCES} )

target_link_libraries(hackrfpp hackrf)
