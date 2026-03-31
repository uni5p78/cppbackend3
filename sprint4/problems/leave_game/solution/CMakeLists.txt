enable_testing()

cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Добавляем библиотеку, указывая, что она статическая.
add_library(ModelLib STATIC     
	src/domain/model.h
    src/domain/model.cpp
    src/domain/loot_generator.h
    src/domain/loot_generator.cpp
    src/domain/tagged.h
	src/domain/collision_detector.cpp
	src/domain/collision_detector.h
	src/domain/geom.h
	src/domain/model_serialization.cpp
	src/domain/model_serialization.h
	src/json/json_loader.cpp
	src/json/json_loader.h
	src/json/json.cpp
	src/json/json.h
	src/json/extra_data.cpp
	src/json/extra_data.h
)

# Добавляем сторонние библиотеки. Указываем видимость PUBLIC, т. к. 
# они должны быть видны и в библиотеке ModelLib, и в зависимостях.
target_include_directories(ModelLib PUBLIC ${ZLIB_INCLUDES})
target_link_libraries(ModelLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})

add_executable(game_server
	src/main.cpp
	src/http/http_server.cpp
	src/http/http_server.h
	src/http/request_handler.cpp
	src/http/request_handler.h
	src/logger/logger.cpp
	src/logger/logger.h
	src/logger/logging_request_handler.cpp
	src/logger/logging_request_handler.h
	src/app/application.cpp
	src/app/application.h
	src/app/tick.cpp
	src/app/tick.h
	src/app/unit_of_work.h
	src/util/sdk.h
	src/util/comand_line.cpp
	src/util/comand_line.h
	src/db/connection_pool.cpp
	src/db/connection_pool.h
	src/db/db.cpp
	src/db/db.h
)

# используем "импортированную" цель CONAN_PKG::boost
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server PRIVATE 
	CONAN_PKG::boost 	
	CONAN_PKG::libpq 
	CONAN_PKG::libpqxx
	Threads::Threads 
	ModelLib
)


add_executable(game_server_tests
    tests/model-tests.cpp
    tests/loot_generator_tests.cpp
	tests/collision-detector-tests.cpp
	tests/state-serialization-tests.cpp
)
target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server_tests PRIVATE 
	CONAN_PKG::catch2 
	CONAN_PKG::boost 
	Threads::Threads
	ModelLib) 
