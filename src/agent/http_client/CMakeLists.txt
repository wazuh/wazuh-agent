cmake_minimum_required(VERSION 3.22)

project(HttpClient)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS asio beast system url)

if(WIN32)
    set(VERIFY_UTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/certificate/https_socket_verify_utils_win.cpp")
    list(APPEND VERIFY_UTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/certificate/https_verifier_win.cpp")
elseif(APPLE)
    set(VERIFY_UTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/certificate/https_socket_verify_utils_mac.cpp")
    list(APPEND VERIFY_UTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/certificate/https_verifier_mac.cpp")
else()
    set(VERIFY_UTILS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/certificate/https_socket_verify_utils_lin.cpp")
endif()

add_library(HttpClient src/http_client.cpp src/http_request_params.cpp ${VERIFY_UTILS_FILE})

target_include_directories(HttpClient
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/certificate)

target_link_libraries(HttpClient PUBLIC Boost::asio PRIVATE OpenSSL::SSL OpenSSL::Crypto Boost::beast Boost::system Boost::url Logger)

if(WIN32)
    target_link_libraries(HttpClient PRIVATE Crypt32)
elseif(APPLE)
    target_link_libraries(HttpClient PRIVATE "-framework Security" "-framework CoreFoundation")
endif()

include(../../cmake/ConfigureTarget.cmake)
configure_target(HttpClient)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
