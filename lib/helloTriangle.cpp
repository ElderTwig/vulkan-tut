#include "helloTriangle.hpp"

#include "glfwUtility.hpp"
#include "vulkanUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <gsl/gsl>

#include <algorithm>
#include <string>

HelloTriangle::HelloTriangle() :
            m_window{glfwUtils::create_window(width, height, false, "test")},
            m_instance{vulkanUtils::create_instance()},
            m_loaderDispatcherPair{
                    vulkanUtils::create_loader_dispatcher_pair(m_instance)},
            m_debugMessenger{vulkanUtils::create_debug_messenger(
                    m_instance,
                    m_loaderDispatcherPair.dispatcher)},
            m_physicalDevice{vulkanUtils::best_device(m_instance)},
            m_logicalDevice{
                    vulkanUtils::create_logical_device(m_physicalDevice)}
{
    std::cerr << m_physicalDevice.getProperties().deviceName.data() << '\n';
}

auto
HelloTriangle::main_loop() -> void
{
    while(glfwWindowShouldClose(m_window.get()) == 0) {
        glfwPollEvents();
    }
}

auto
HelloTriangle::cleanup() -> void
{
    glfwTerminate();
}
