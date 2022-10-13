cmake_minimum_required(VERSION 3.16.3)

project(Trokam)

set(CMAKE_CXX_STANDARD 17)

set(SUBSYSTEM "ALL" CACHE STRING "Choose a system: WEB, CRAWLER, CONVEYOR or ALL")

message(STATUS "System choice:" ${SUBSYSTEM})

if((SUBSYSTEM STREQUAL "WEB") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")
    include_directories(include)

    file(GLOB SOURCES_WEB    "src_web/*.cpp")
    file(GLOB SOURCES_COMMON "src_common/*.cpp")

    add_executable(search.wt ${SOURCES_WEB} ${SOURCES_COMMON})

    find_package(Boost 1.67 COMPONENTS filesystem regex program_options signals system thread)
    find_package(fmt)

    target_link_libraries(search.wt xapian Boost::system Boost::regex Boost::program_options pqxx wt wtfcgi fmt::fmt)

    install(TARGETS search.wt DESTINATION /var/www/html/bin/)
    install(DIRECTORY etc/style DESTINATION /var/www/html)
    install(DIRECTORY etc/approot DESTINATION /usr/local/etc/trokam)

endif()

if((SUBSYSTEM STREQUAL "CRAWLER") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")
    include_directories(include)

    file(GLOB SOURCES_CMD    "src_cmd/*.cpp")
    file(GLOB SOURCES_COMMON "src_common/*.cpp")

    add_executable(trokam ${SOURCES_CMD} ${SOURCES_COMMON})

    find_package(Boost 1.67 COMPONENTS filesystem regex program_options signals system thread)

    target_link_libraries(trokam curl pqxx xapian Boost::regex Boost::program_options textcat)

    install(TARGETS trokam DESTINATION /usr/local/bin)
    install(FILES etc/language.conf DESTINATION /usr/local/etc/trokam)
    install(DIRECTORY etc/signature DESTINATION /usr/local/etc/trokam)

endif()

if((SUBSYSTEM STREQUAL "CONVEYOR") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3 -pthread")
    include_directories(include)

    #file(GLOB SOURCES_CONVEYOR  "src_conveyor/*.cpp")
    file(GLOB SOURCES_TRANSPORT "src_transport/*.cpp")
    file(GLOB SOURCES_COMMON    "src_common/*.cpp")

    add_executable(conveyor src_conveyor/main.cpp       ${SOURCES_TRANSPORT} ${SOURCES_COMMON})
    add_executable(prime    src_conveyor/main_prime.cpp ${SOURCES_TRANSPORT} ${SOURCES_COMMON})

    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

    find_package(Boost 1.67 COMPONENTS filesystem regex program_options signals system thread log)
    find_package(fmt)

    target_link_libraries(conveyor curl pqxx xapian Boost::system Boost::regex Boost::filesystem Boost::program_options Boost::log fmt::fmt)
    target_link_libraries(prime curl pqxx xapian Boost::system Boost::regex Boost::filesystem Boost::program_options Boost::log fmt::fmt)

    install(TARGETS conveyor DESTINATION /usr/local/bin)
    install(TARGETS prime DESTINATION /usr/local/bin)

endif()

if((SUBSYSTEM STREQUAL "CLEANER") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3 -pthread")
    include_directories(include)

    file(GLOB SOURCES_CLEANER   "src_cleaner/*.cpp")
    file(GLOB SOURCES_TRANSPORT "src_transport/*.cpp")
    file(GLOB SOURCES_COMMON    "src_common/*.cpp")

    add_executable(cleaner ${SOURCES_CLEANER} ${SOURCES_TRANSPORT} ${SOURCES_COMMON})

    ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

    find_package(Boost 1.67 COMPONENTS filesystem regex program_options signals system thread log)
    find_package(fmt)

    target_link_libraries(cleaner curl pqxx xapian Boost::system Boost::regex Boost::filesystem Boost::program_options Boost::log fmt::fmt)

    install(TARGETS cleaner DESTINATION /usr/local/bin)

endif()
