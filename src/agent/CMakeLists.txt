cmake_minimum_required(VERSION 3.22)

project(Agent)

include(../cmake/CommonSettings.cmake)
set_common_settings()

add_subdirectory(agent_info)
add_subdirectory(centralized_configuration)
add_subdirectory(command_handler)
add_subdirectory(communicator)
add_subdirectory(configuration_parser)
add_subdirectory(http_client)
add_subdirectory(instance_communicator)
add_subdirectory(multitype_queue)
add_subdirectory(persistence)
add_subdirectory(task_manager)
add_subdirectory(restart_handler)

find_package(Boost REQUIRED COMPONENTS asio system program_options)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt REQUIRED)

if(WIN32)
    set(SOURCES src/agent_runner_win.cpp src/windows/windows_api_facade.cpp src/windows/windows_service.cpp
                src/signal_handler_win.cpp src/instance_handler_win.cpp)
elseif(UNIX)
    set(SOURCES src/agent_runner_unix.cpp src/signal_handler_unix.cpp src/instance_handler_unix.cpp)
endif()

list(
    APPEND
    SOURCES
    src/agent.cpp
    src/agent_enrollment.cpp
    src/agent_runner.cpp
    src/command_handler_utils.cpp
    src/message_queue_utils.cpp
    src/signal_handler.cpp
    src/instance_handler.cpp)

add_library(Agent ${SOURCES})
target_include_directories(
    Agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/unix
            ${CMAKE_CURRENT_SOURCE_DIR}/src/windows)
target_link_libraries(
    Agent
    PUBLIC AgentInfo
           CentralizedConfiguration
           CommandHandler
           Communicator
           ConfigurationParser
           HttpClient
           InstanceCommunicator
           MultiTypeQueue
           ModuleManager
           TaskManager
           RestartHandler
           sysinfo
    PRIVATE Boost::asio
            Boost::system
            Boost::program_options
            nlohmann_json::nlohmann_json
            fmt::fmt
            FilesystemWrapper
            Logger
            Config)

include(../cmake/ConfigureTarget.cmake)
configure_target(Agent)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
