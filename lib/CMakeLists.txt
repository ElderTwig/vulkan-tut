file(GLOB SRC
    "*.cpp")

add_library(vulkan-app_lib STATIC)
    
target_sources(vulkan-app_lib 
    PUBLIC ${SRC})

target_include_directories(vulkan-app_lib
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(vulkan-app_lib)

set_target_properties(vulkan-app_lib
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_target_properties(vulkan-app_lib
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CLANG_TIDY)
    set_target_properties(vulkan-app PROPERTIES
        CXX_CLANG_TIDY "clang-tidy;-header-filter=./include")
endif()

if(MSVC)
    target_compile_options(vulkan-app_lib
        PRIVATE /W4)
else()
    target_compile_options(vulkan-app_lib
        PRIVATE -Wall
        PRIVATE -Werror
        PRIVATE -Wextra)
endif()

add_library(vulkan-app::lib ALIAS ${APP_NAME}_lib)
