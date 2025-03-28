# CMake lowest version requirement
cmake_minimum_required(VERSION 3.5.1)

# project information
project(ACL_DVPP)

# Compile options
add_compile_options(-std=c++11)

# include path
include_directories (../include)
include_directories("/home/dml/Ascend/ascend-toolkit/latest/include")

# add src files
aux_source_directory(. SRC_LIST)

add_executable(main ${SRC_LIST})
