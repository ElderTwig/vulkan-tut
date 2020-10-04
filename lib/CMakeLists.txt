file(GLOB SRC
    "*.cpp")

add_library(${APP_NAME}_lib STATIC)
    
target_sources(${APP_NAME}_lib 
    PUBLIC ${SRC})

target_include_directories(${APP_NAME}_lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${APP_NAME}_lib)

set_target_properties(${APP_NAME}_lib
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_target_properties(${APP_NAME}_lib
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CLANG_TIDY)
    set_target_properties(${APP_NAME} PROPERTIES
        CXX_CLANG_TIDY "clang-tidy;-header-filter=./include")
endif()

if(MSVC)
    target_compile_options(${APP_NAME}_lib
        PRIVATE /W4)
else()
    target_compile_options(${APP_NAME}_lib
        PRIVATE -Wall
        PRIVATE -Werror
        PRIVATE -Wextra)
endif()

add_library(${APP_NAME}::lib ALIAS ${APP_NAME}_lib)
