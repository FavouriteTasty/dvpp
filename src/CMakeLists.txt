# CMake lowest version requirement
cmake_minimum_required(VERSION 3.10)

# project information
project(ACL_DVPP)

# Compile options
add_compile_options(-std=c++11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "out")
set(CMAKE_CXX_FLAGS_DEBUG "-fPIC -O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-fPIC -O2 -Wall")
add_definitions(-DENABLE_DVPP_INTERFACE)

# include path
include_directories (../inc)
include_directories("/home/dml/Ascend/ascend-toolkit/latest/include")

# .so path
link_directories("/home/dml/Ascend/ascend-toolkit/latest/lib64")

# add src files
aux_source_directory(. SRC_LIST)

add_executable(main ${SRC_LIST})

# ffmpeg
set(FFMPEG_DIR "/home/dml/ffmpeg/ffmpeg-ascend/ascend") 
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include")
set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
find_library(AVFORMAT_LIB avformat HINTS ${FFMPEG_LIB_DIR})
find_library(AVCODEC_LIB avcodec HINTS ${FFMPEG_LIB_DIR})
find_library(AVUTIL_LIB avutil HINTS ${FFMPEG_LIB_DIR})
find_library(SWRESAMPLE_LIB swresample HINTS ${FFMPEG_LIB_DIR})

find_package(Threads REQUIRED)

target_link_libraries(main PRIVATE
    ascendcl 
    stdc++ 
    acl_dvpp 
    Threads::Threads
    ${AVFORMAT_LIB}
    ${AVCODEC_LIB}
    ${AVUTIL_LIB}
    ${SWRESAMPLE_LIB}
    )
