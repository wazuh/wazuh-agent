cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

include(../cmake/CommonSettings.cmake)
set_common_settings()

add_subdirectory(agent_info)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)
add_subdirectory(multitype_queue)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)

set(SOURCES
    src/agent.cpp
    src/message_queue_utils.cpp
    src/register.cpp
    src/signal_handler.cpp
    $<$<BOOL:$<PLATFORM_ID:Windows>>:src/signal_handler_win.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/signal_handler_unix.cpp>
    src/task_manager.cpp
)

add_library(Agent ${SOURCES})
target_include_directories(Agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Agent PUBLIC ConfigurationParser Communicator AgentInfo MultiTypeQueue PRIVATE OpenSSL::SSL OpenSSL::Crypto Boost::asio Boost::beast)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
