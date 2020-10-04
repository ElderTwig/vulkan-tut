add_executable(${APP_NAME} main.cpp)

target_link_libraries(${APP_NAME}
    PRIVATE ${APP_NAME}::lib)

set_target_properties(${APP_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_target_properties(${APP_NAME}
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CLANG_TIDY)
    set_target_properties(${APP_NAME} PROPERTIES
        CXX_CLANG_TIDY "clang-tidy;-header-filter=./include")
endif()

if(MSVC)
    target_compile_options(${APP_NAME}
        PRIVATE /W4)
else()
    target_compile_options(${APP_NAME}
        PRIVATE -Wall
        PRIVATE -Werror
        PRIVATE -Wextra)
endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL Release)
  target_link_options(${APP_NAME}
        PRIVATE /SUBSYSTEM:WINDOWS
        PRIVATE /ENTRY:mainCRTStartup)
endif()
