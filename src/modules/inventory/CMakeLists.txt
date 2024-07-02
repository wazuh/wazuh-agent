add_library(Inventory src/inventory.cpp)
target_include_directories(Inventory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
