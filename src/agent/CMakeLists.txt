cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/../)

project(Agent)

include(../cmake/CommonSettings.cmake)
set_common_settings()

include_directories(${CMAKE_SOURCE_DIR}/common/logger/include)

add_subdirectory(agent_info)
add_subdirectory(command_handler)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)
add_subdirectory(module_command)
add_subdirectory(multitype_queue)
add_subdirectory(sqlite_manager)
add_subdirectory(command_store)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast)


if(WIN32)
    set(SOURCES
        src/windows_service.cpp
        src/process_options_win.cpp
    )
elseif(UNIX)
    set(SOURCES
        src/unix_daemon.cpp
        src/process_options_unix.cpp
    )
endif()

list(APPEND SOURCES
    src/agent.cpp
    src/agent_registration.cpp
    src/command_handler_utils.cpp
    src/message_queue_utils.cpp
    src/process_options.cpp
    src/signal_handler.cpp
    $<$<BOOL:$<PLATFORM_ID:Windows>>:src/signal_handler_win.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/signal_handler_unix.cpp>
    src/task_manager.cpp
)

add_library(Agent ${SOURCES})
target_include_directories(Agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Agent
    PUBLIC
    ConfigurationParser
    Communicator
    AgentInfo
    CommandHandler
    MultiTypeQueue
    ModuleManager
    ModuleCommand
    CentralizedConfiguration
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::asio
    Boost::beast
)

include(../cmake/ConfigureTarget.cmake)
configure_target(Agent)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
