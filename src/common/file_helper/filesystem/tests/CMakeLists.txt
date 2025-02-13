find_package(GTest CONFIG REQUIRED)

include(../../../../cmake/ConfigureTarget.cmake)

add_executable(filesystem_test filesystem_test.cpp)
configure_target(filesystem_test)
target_include_directories(filesystem_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(filesystem_test PRIVATE FilesystemWrapper GTest::gtest)
add_test(NAME Filesystem COMMAND filesystem_test)
