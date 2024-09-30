add_library(utils INTERFACE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../logger/include)

target_include_directories(utils INTERFACE include Logger)
