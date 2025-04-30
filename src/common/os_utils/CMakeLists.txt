cmake_minimum_required(VERSION 3.22)

project(OsUtils)

include(../../cmake/CommonSettings.cmake)
set_common_settings()

add_library(OsUtils $<$<PLATFORM_ID:Linux>:src/os_utils_unix.cpp> $<$<PLATFORM_ID:Darwin>:src/os_utils_macos.cpp>
                    $<$<PLATFORM_ID:Windows>:src/os_utils_win.cpp>)
target_include_directories(OsUtils PUBLIC include PRIVATE src)
target_link_libraries(OsUtils PUBLIC FilesystemWrapper PRIVATE Logger deleter_helper)

include(../../cmake/ConfigureTarget.cmake)
configure_target(OsUtils)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
