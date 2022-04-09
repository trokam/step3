cmake_minimum_required(VERSION 3.16.3)

project(Trokam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")

add_executable(
    trokam
    crawler.cpp
    doc_processor.cpp
    file_ops.cpp
    main.cpp
    options.cpp
    postgresql.cpp
    warehouse.cpp)

find_package(Boost 1.67 COMPONENTS filesystem regex program_options) 

target_link_libraries(trokam curl pqxx Boost::regex Boost::program_options)
