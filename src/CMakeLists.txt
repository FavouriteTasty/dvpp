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

find_package(Threads REQUIRED)
target_link_libraries(main ascendcl stdc++ acl_dvpp Threads::Threads)
