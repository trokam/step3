cmake_minimum_required(VERSION 3.16.3)

project(Trokam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")

add_executable(index_web main.cpp crawler.cpp doc_processor.cpp warehouse.cpp postgresql.cpp)
target_link_libraries(index_web curl pqxx)
