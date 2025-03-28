cmake_minimum_required(VERSION 3.22)

project(unit_tests)

find_package(GTest CONFIG REQUIRED)

add_subdirectory(inventory)
add_subdirectory(inventoryImp)
add_subdirectory(invNormalizer)
add_subdirectory(statelessEvent)
