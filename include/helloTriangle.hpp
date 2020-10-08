#ifndef VK_TUT_HELLO_TRIANGLE_HPP
#define VK_TUT_HELLO_TRIANGLE_HPP

#include "glfwUtility.hpp"
#include "vulkanUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>

class HelloTriangle {
public:
    static int constexpr width  = 800;
    static int constexpr height = 600;

    HelloTriangle();

    auto
    run() -> void
    {
        main_loop();
        cleanup();
    }

private:
    glfwUtils::UniqueWindow m_window;
    vk::UniqueInstance m_instance;
    vulkanUtils::LoaderDispatcherPair m_loaderDispatcherPair;
    vulkanUtils::UniqueDebugUtilsMessengerEXT m_debugMessenger;

    auto
    main_loop() -> void;

    static auto
    cleanup() -> void;
};

#endif    // VK_TUT_HELLO_TRIANGLE_HPP
