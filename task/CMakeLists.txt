cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(queue_tasks)

set(main main)
set(queue_tasks queue_tasks)

find_package(MPI)
include_directories(${MPI_INCLUDE_PATH}) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/queue_tasks)

add_subdirectory(main)
add_subdirectory(queue_tasks)
