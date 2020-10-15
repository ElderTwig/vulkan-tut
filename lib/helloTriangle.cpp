#include "helloTriangle.hpp"

#include "glfwUtility.hpp"
#include "vulkanUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <gsl/gsl>

#include <algorithm>
#include <string>

[[nodiscard]] auto
create_index_list(
        vulkanUtils::QueueFamilyAndPos const& graphics,
        vulkanUtils::QueueFamilyAndPos const& presentation)
        -> std::vector<uint32_t>
{
    auto const duplicateQueues = graphics.position == presentation.position;

    if(duplicateQueues) {
        return {};
    }

    return {static_cast<unsigned int>(graphics.position),
            static_cast<unsigned int>(presentation.position)};
}

HelloTriangle::HelloTriangle() :
            m_window{glfwUtils::create_window(width, height, false, "test")},
            m_instance{vulkanUtils::create_instance(m_validationLayers)},
            m_loaderDispatcherPair{
                    vulkanUtils::create_loader_dispatcher_pair(m_instance)},
            m_debugMessenger{vulkanUtils::create_debug_messenger(
                    m_instance,
                    m_loaderDispatcherPair.dispatcher)},
            m_surface{glfwUtils::create_window_surface(m_instance, m_window)},
            m_physicalDevice{
                    vulkanUtils::best_device(m_instance, m_deviceExtensions)},
            m_graphicsQueues{
                    vulkanUtils::get_graphics_queues(m_physicalDevice)},
            m_presentationQueues{vulkanUtils::get_present_queues(
                    m_physicalDevice,
                    m_graphicsQueues,
                    m_surface)},
            m_queuePriorities(m_graphicsQueues.properties.queueCount, 1.0f),
            m_queueIndicies{
                    create_index_list(m_graphicsQueues, m_presentationQueues)},
            m_logicalDevice{vulkanUtils::create_logical_device(
                    m_physicalDevice,
                    m_graphicsQueues,
                    m_queuePriorities,
                    m_validationLayers,
                    m_deviceExtensions)},
            m_deviceQueue{
                    m_logicalDevice->getQueue(m_graphicsQueues.position, 0)},
            m_swapChain(vulkanUtils::create_swap_chain(
                    m_physicalDevice,
                    m_surface,
                    m_logicalDevice,
                    {width, height},
                    {vulkanUtils::defaultSurfaceFormat},
                    {vk::PresentModeKHR::eFifoRelaxed,
                     vk::PresentModeKHR::eFifo},
                    m_queueIndicies)),
            m_swapChainImages(
                    m_logicalDevice->getSwapchainImagesKHR(*m_swapChain))
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
