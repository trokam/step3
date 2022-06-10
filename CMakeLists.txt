cmake_minimum_required(VERSION 3.16.3)

project(Trokam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")

add_executable(
    trokam
    crawler.cpp
    doc_processor.cpp
    file_ops.cpp
    grasp.cpp
    main.cpp
    options.cpp
    plain_text_processor.cpp
    postgresql.cpp
    warehouse.cpp)

find_package(Boost 1.67 COMPONENTS filesystem regex program_options) 

target_link_libraries(trokam curl pqxx xapian Boost::regex Boost::program_options textcat)

install(TARGETS trokam DESTINATION /usr/local/bin)
install(FILES etc/language.conf DESTINATION /usr/local/etc/trokam)
install(DIRECTORY etc/signature DESTINATION /usr/local/etc/trokam)
