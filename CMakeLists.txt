cmake_minimum_required (VERSION 2.6)
add_compile_options(-std=c++11)

include_directories(include include/internal)
set(INTERNAL_HEADER_FILES include/internal/kendall.h include/internal/pprInternal.h 
include/internal/pprSingleSource.h)
set(HEADER_FILES include/grank.h include/benchmarkAlgorithm.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -march=native")
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
project (ppr)

#########main build
add_executable(ppr src/main.cc ${HEADER_FILES} ${INTERNAL_HEADER_FILES})



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


#########test
include_directories(googletest-src/googletest/include/gtest)
add_executable(pprTest test/internal/jaccardTest.cc test/internal/keepTopTest.cc test/internal/norm1Test.cc test/internal/findPartitionsTest.cc 
test/internal/pprSingleSourceTest.cc test/grankTest.cc test/benchmarkAlgorithmTest.cc
test/grankHeaderOnlyTest.cc
${HEADER_FILES} ${INTERNAL_HEADER_FILES})
target_link_libraries(pprTest gtest gtest_main)
add_test(pprTest pprTest)
#make it so that "make tests" will build pprTest and gtest
add_custom_target(tests)
add_dependencies(tests pprTest)
add_dependencies(pprTest gtest gtest_main gmock gmock_main)
#exclude pprTest and gtest stuff from "make"
set_target_properties(pprTest gtest gtest_main gmock gmock_main PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
