# CMAKEFILE REF: 
#   https://blog.csdn.net/FPGATOM/article/details/104696543
#   https://www.cnblogs.com/chaofn/p/10160555.html
cmake_minimum_required(VERSION 3.12)

project(ycsbcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

find_library(LIB_ROCKSDB NAMES rocksdb PATHS /home/hank/.local/lib NO_DEFAULT_PATH)
message("Using RockDB Lib: " ${LIB_ROCKSDB})

set(CORE_DIR ./core)
set(ROCKSDB_DIR ./rocksdb)

include_directories(.)

aux_source_directory(./core SOURCES)
aux_source_directory(./rocksdb SOURCES)
add_executable(ycsb ${SOURCES})

target_link_libraries(ycsb ${LIB_ROCKSDB} snappy z lz4 bz2 zstd dl pthread)
