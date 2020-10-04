add_executable(vkTut main.cpp)

target_link_libraries(vkTut
    PRIVATE vkTut::lib)

set_target_properties(vkTut
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_target_properties(vkTut
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CLANG_TIDY)
    set_target_properties(vkTut PROPERTIES
        CXX_CLANG_TIDY "clang-tidy;-header-filter=./include")
endif()

if(MSVC)
    target_compile_options(vkTut
        PRIVATE /W4)
else()
    target_compile_options(vkTut
        PRIVATE -Wall
        PRIVATE -Werror
        PRIVATE -Wextra)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL Release)
  target_link_options(vkTut
        PRIVATE /SUBSYSTEM:WINDOWS
        PRIVATE /ENTRY:mainCRTStartup)
endif()
