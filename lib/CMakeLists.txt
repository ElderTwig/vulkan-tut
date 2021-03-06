file(GLOB SRC
    "*.cpp")

add_library(vkTut_lib SHARED)
    
target_sources(vkTut_lib 
    PRIVATE ${SRC})

target_include_directories(vkTut_lib
    PUBLIC ../include)

target_link_libraries(vkTut_lib
    PUBLIC
        Vulkan::Vulkan
        glfw::glfw
        glm::glm
        Microsoft.GSL::GSL
        ${CMAKE_DL_LIBS})

set_target_properties(vkTut_lib
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set_target_properties(vkTut_lib
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(CLANG_TIDY)
    set_target_properties(vkTut_lib PROPERTIES
        CXX_CLANG_TIDY "clang-tidy;-header-filter=./include")
endif()

if(MSVC)
    target_compile_options(vkTut_lib
        PRIVATE /W4)
else()
    target_compile_options(vkTut_lib
        PRIVATE -Wall
        PRIVATE -Werror
        PRIVATE -Wextra)
endif()

add_library(vkTut::lib ALIAS vkTut_lib)
