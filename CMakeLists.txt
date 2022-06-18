cmake_minimum_required(VERSION 3.16.3)

project(Trokam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O3")

#set(SOURCES_WEB src)
include_directories(include)

file(GLOB SOURCES_WEB    "src_web/*.cpp")
file(GLOB SOURCES_CMD    "src_cmd/*.cpp")
file(GLOB SOURCES_COMMON "src_common/*.cpp")

add_executable(search.wt ${SOURCES_WEB} ${SOURCES_COMMON})
add_executable(trokam    ${SOURCES_CMD} ${SOURCES_COMMON})

#find_package(Boost 1.67 COMPONENTS filesystem regex program_options)
find_package(Boost 1.67 COMPONENTS filesystem regex program_options signals system thread)

target_link_libraries(trokam curl pqxx xapian Boost::regex Boost::program_options textcat)
target_link_libraries(search.wt xapian Boost::system Boost::regex Boost::program_options pqxx wt wtfcgi)

install(TARGETS trokam DESTINATION /usr/local/bin)
install(FILES etc/language.conf DESTINATION /usr/local/etc/trokam)
install(DIRECTORY etc/signature DESTINATION /usr/local/etc/trokam)

install(TARGETS search.wt DESTINATION /var/www/html/bin/)
install(DIRECTORY etc/style DESTINATION /var/www/html/)
