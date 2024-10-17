function(set_common_settings)
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

    option(BUILD_TESTS "Enable tests building" OFF)
    option(COVERAGE "Enable coverage report" OFF)
    option(ENABLE_LOGCOLLECTOR "Enable Logcollector module" ON)

    if(COVERAGE)
        if(NOT TARGET coverage)
            set(CMAKE_BUILD_TYPE Debug)

            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                add_compile_options(-O0 -fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
                add_link_options(-fprofile-arcs -ftest-coverage -fprofile-instr-generate -fcoverage-mapping)
            else()
                add_compile_options(-O0 --coverage) # note that -g3 is required for gcov but it's already below
                add_link_options(--coverage)
            endif()

            # We found that gcc gcov doesn't report coverage on
            # coroutines, so we use llvm-cov if available
            find_program(LLVM_COV_EXECUTABLE NAMES llvm-cov)

            if(LLVM_COV_EXECUTABLE)
                set(GCOV_EXECUTABLE "llvm-cov gcov")
            else()
                set(GCOV_EXECUTABLE "gcov")
            endif()

            add_custom_target(coverage
                COMMAND ${CMAKE_MAKE_PROGRAM} test
                COMMAND gcovr --gcov-executable "${GCOV_EXECUTABLE}" -v -r .. -e '.*main\.cpp' -e '.*vcpkg.*' -e '.*mock_.*' -e '.*_test.*' --html --html-details -o coverage.html
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report"
            )
        endif()
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" PARENT_SCOPE)
    endif()

    if(MSVC)
        set(CMAKE_CXX_FLAGS "/Zi /DWIN32 /EHsc" PARENT_SCOPE)
        add_compile_options(/Zc:preprocessor)
    else()
        set(CMAKE_CXX_FLAGS "-g3" PARENT_SCOPE)
    endif()

    if(ENABLE_LOGCOLLECTOR)
        add_definitions(-DENABLE_LOGCOLLECTOR)
    endif()
endfunction()
