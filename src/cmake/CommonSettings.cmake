function(set_common_settings)
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" PARENT_SCOPE)
    endif()

    if(MSVC)
        set(CMAKE_CXX_FLAGS "/Zi /DWIN32 /EHsc" PARENT_SCOPE)
    else()
        set(CMAKE_CXX_FLAGS "-g3" PARENT_SCOPE)
    endif()
endfunction()
