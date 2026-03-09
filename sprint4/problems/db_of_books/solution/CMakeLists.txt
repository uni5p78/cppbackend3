cmake_minimum_required(VERSION 3.11)

project(book_manager CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
  
find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED) 

add_executable(book_manager 
    src/main.cpp
    src/json.cpp
    src/json.h
    src/sql_server.cpp
    src/sql_server.h
    src/cmd_manager.h
    src/cmd_manager.cpp
    src/request_handler.h
    src/request_handler.cpp
    
)

target_include_directories(book_manager PRIVATE CONAN_PKG::boost)
target_link_libraries(book_manager PRIVATE CONAN_PKG::boost CONAN_PKG::libpqxx Threads::Threads)