cmake_minimum_required(VERSION 3.1.0)
project(radapter-broker)
find_package(Qt5 REQUIRED COMPONENTS Core Concurrent)
add_compile_options(-std=c++11)

include_directories(
  src
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Concurrent_INCLUDE_DIRS}
)

add_library(json-formatters
    src/json-formatters/formatters/dict.cpp
    src/json-formatters/formatters/jsondict.cpp
    src/json-formatters/formatters/list.cpp
)

target_link_libraries(json-formatters
  Qt5::Core
)

add_library(radapter-broker
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