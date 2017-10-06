cmake_minimum_required(VERSION 3.7)
project(tabun_api)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES user.cpp comments.cpp)
set(HEADERS libtabun.h)
add_library(tabun_api STATIC ${SOURCE_FILES} ${HEADERS})

target_link_libraries(tabun_api curl)

target_link_libraries(tabun_api htmlcxx)
