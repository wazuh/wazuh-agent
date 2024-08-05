cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

set(SOURCES
    src/agent.cpp
    src/task_manager.cpp
    src/register.cpp
    src/signal_handler.cpp
)

set(HEADERS
    include/agent.hpp
    include/itask_manager.hpp
    include/task_manager.hpp
    include/register.hpp
    include/signal_handler.hpp
)

add_subdirectory(agent_info)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)

find_package(Boost REQUIRED COMPONENTS asio)

add_library(agent ${SOURCES} ${HEADERS})
target_include_directories(agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(agent PUBLIC ConfigurationParser communicator AgentInfo PRIVATE ${Boost_LIBRARIES})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
