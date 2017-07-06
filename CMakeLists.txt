cmake_minimum_required (VERSION 2.6)
add_compile_options(-std=c++11)

include_directories(include include/internal header-only)
set(INTERNAL_HEADER_FILES include/internal/kendall.h include/internal/pprInternal.h 
include/internal/pprSingleSource.h)
set(HEADER_FILES include/grank.h include/benchmarkAlgorithm.h include/mccompletepathv2.h header-only/grankMulti.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -march=native -pthread")
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
project (ppr)

#########main build
add_executable(ppr src/main.cc ${HEADER_FILES} ${INTERNAL_HEADER_FILES})

