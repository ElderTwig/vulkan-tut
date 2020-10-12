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
    vk::UniqueSurfaceKHR m_surface;

    std::vector<char const*> const m_deviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vk::PhysicalDevice m_physicalDevice;

    vulkanUtils::QueueFamilyAndPos m_graphicsQueues;
    vulkanUtils::QueueFamilyAndPos m_presentationQueues;
    std::vector<float> const m_queuePriorities;

    vk::UniqueDevice m_logicalDevice;
    vk::Queue m_deviceQueue;

    auto
    main_loop() -> void;

    static auto
    cleanup() -> void;
};

#endif    // VK_TUT_HELLO_TRIANGLE_HPP
