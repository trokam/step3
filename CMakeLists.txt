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

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    #target_link_libraries(search.wt PRIVATE Threads::Threads)

    find_package(
        Boost 1.67 COMPONENTS
        filesystem
        regex
        program_options
        signals
        system
        thread)
    find_package(fmt)

    target_link_libraries(
        search.wt
        xapian
        Boost::system
        Boost::regex
        Boost::program_options
        pqxx
        wt
        wtfcgi
        fmt::fmt
        Threads::Threads)

    install(TARGETS search.wt DESTINATION /var/www/trokam/bin/)
    install(DIRECTORY etc/info DESTINATION /var/www/trokam)
    install(DIRECTORY etc/style DESTINATION /var/www/trokam)
    install(DIRECTORY etc/approot DESTINATION /usr/local/etc/trokam)
    install(FILES etc/words.json DESTINATION /usr/local/etc/trokam)

endif()

if((SUBSYSTEM STREQUAL "CRAWLER") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")
    include_directories(include)
    file(GLOB SOURCES_CMD    "src_crawler/*.cpp")
    file(GLOB SOURCES_COMMON "src_common/*.cpp")
    add_executable(trokam ${SOURCES_CMD} ${SOURCES_COMMON})

    find_package(
        Boost 1.67 COMPONENTS
        filesystem
        regex
        program_options
        signals
        system
        thread)
    find_package(fmt)

    target_link_libraries(
        trokam
        curl
        pqxx
        xapian
        Boost::regex
        Boost::program_options
        exttextcat
        fmt::fmt)

    install(TARGETS trokam DESTINATION /usr/local/bin)
    install(FILES etc/language.conf DESTINATION /usr/local/etc/trokam)
    install(DIRECTORY etc/signature DESTINATION /usr/local/etc/trokam)

endif()

if((SUBSYSTEM STREQUAL "PUMP") OR (SUBSYSTEM STREQUAL "ALL"))

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3 -pthread")
    include_directories(include)
    file(GLOB SOURCES_CONVEYOR   "src_pump/*.cpp")
    file(GLOB SOURCES_COMMON     "src_common/*.cpp")
    add_executable(pump src_pump/main.cpp  ${SOURCES_TRANSPORT} ${SOURCES_COMMON})
    add_definitions(-DBOOST_LOG_DYN_LINK)

    find_package(
        Boost 1.67 COMPONENTS
        filesystem
        regex
        program_options
        signals
        system
        thread
        log)

    find_package(fmt)

    target_link_libraries(
        pump
        pqxx
        xapian
        Boost::system
        Boost::regex
        Boost::filesystem
        Boost::program_options
        Boost::log
        fmt::fmt)

    install(TARGETS pump DESTINATION /usr/local/bin)

endif()
