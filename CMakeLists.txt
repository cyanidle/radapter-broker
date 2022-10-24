cmake_minimum_required(VERSION 3.1.0)
project(radapter_broker)
find_package(Qt5 REQUIRED COMPONENTS Core Concurrent)
add_compile_options(-std=c++11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")

include_directories(
  ${radapter_broker_SOURCE_DIR}/src
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Concurrent_INCLUDE_DIRS}
)

message("== INCLUDE DIRS: ${radapter_broker_SOURCE_DIR}/src ==")

add_library(json-formatters STATIC
    src/json-formatters/formatters/dict.cpp
    src/json-formatters/formatters/jsondict.cpp
    src/json-formatters/formatters/list.cpp
    src/json-formatters/logging/jsonformatterslogging.cpp
  )

target_link_libraries(json-formatters 
  Qt5::Core
)

add_library(radapter-broker STATIC
    src/radapter-broker/broker.cpp
    src/radapter-broker/brokerlogging.cpp
    src/radapter-broker/factorybase.cpp
    src/radapter-broker/interceptorbase.cpp
    src/radapter-broker/singletonbase.cpp
    src/radapter-broker/workerbase.cpp
    src/radapter-broker/workermsg.cpp
    src/radapter-broker/workerproxy.cpp
)

target_link_libraries(radapter-broker
  Qt5::Core
  json-formatters
)

set(${PROJECT_NAME}_INCLUDE_DIRS INCLUDE_DIRECTORIES)