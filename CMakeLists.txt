cmake_minimum_required (VERSION 2.6)
add_compile_options(-std=c++11)

include_directories(include)
set(HEADER_FILES include/kendall.h include/grank.h include/pprInternal.h 
include/pprSingleSource.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -flto -march=native")
project (ppr)

add_executable(ppr src/main.cc ${HEADER_FILES})

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )


# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in
               googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )


# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include"
                      "${gmock_SOURCE_DIR}/include")
endif()

#build test
#add_executable(probTest test/CountMinSketchTest.cc src/MurmurHash3.cc)
#target_link_libraries(probTest gtest gtest_main)
#add_test(probTest probTest)
