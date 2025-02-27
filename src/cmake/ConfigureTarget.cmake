function(configure_target target)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(common_warnings
            -Wall
            -Wextra
            -Werror
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wformat=2
            -Wfloat-equal
            -Wundef
            -Wcast-align
            -Wswitch-enum
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Woverloaded-virtual
            -Wmissing-declarations
            -Wredundant-decls
            -Wpessimizing-move
            -Wsuggest-override
        )
        target_compile_options(${target} PRIVATE ${common_warnings})
    else()
        set(msvc_warnings
            /W4
            /WX
            /permissive-
            /sdl
            /Wv:18 # Additional security-related warnings and runtime checks
            /w14640 # Conversion from 'int' to 'float', possible loss of data
            /w14996 # Conversion from 'size_t' to 'unsigned int', possible loss of data
            /w14955 # Declaration hides previous local declaration
            /w14546 # Function pointer conversion to a different size
            /w14547 # Function pointer conversion to a different calling convention
        )
        add_definitions(-D_WIN32_WINNT=0x0601)
        target_compile_options(${target} PRIVATE ${msvc_warnings})
    endif()

    option(ENABLE_CLANG_TIDY "Enable clang-tidy analysis" ON)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND ENABLE_CLANG_TIDY)
        find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy-18)

        if(CLANG_TIDY_EXECUTABLE)
            set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
        else()
            message(WARNING "Not Found: clang-tidy-18")
        endif()
    endif()
endfunction()
