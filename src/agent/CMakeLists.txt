cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

include_directories(include)

set(SOURCES
    src/agent.cpp
    src/message_task.cpp
    src/task_manager.cpp
    src/register.cpp
)

set(HEADERS
    include/agent.hpp
    include/itask_manager.hpp
    include/task_manager.hpp
    include/message_task.hpp
    include/register.hpp
)

add_library(agent ${SOURCES} ${HEADERS})

find_package(Boost REQUIRED COMPONENTS asio)

target_include_directories(agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/communicator/include)

# Add the subdirectory for each child project
add_subdirectory(agent_info)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)

target_link_libraries(agent PUBLIC ConfigurationParser PRIVATE ${Boost_LIBRARIES} communicator AgentInfo)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
