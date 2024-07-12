cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

include_directories(include)

set(SOURCES
    src/agent.cpp
    src/message_task.cpp
    src/task_manager.cpp
)

set(HEADERS
    include/agent.hpp
    include/itask_manager.hpp
    include/task_manager.hpp
    include/message_task.hpp
)

add_library(agent ${SOURCES} ${HEADERS})

add_subdirectory(configuration_parser)

find_package(Boost REQUIRED COMPONENTS asio)

target_link_libraries(agent PRIVATE ${Boost_LIBRARIES})

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
