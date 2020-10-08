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
            m_instance{create_instace()}
{}

auto
HelloTriangle::create_instace() -> vk::UniqueInstance
{
    auto const requiredExtensions = glfwUtils::required_vk_extensions();

    if(!vulkanUtility::extensions_supported(requiredExtensions)) {
        throw std::runtime_error("Required extensions not supported!");
    }

    auto const supportedLayers = std::vector{"VK_LAYER_KHRONOS_validation"};
    if(!vulkanUtility::layers_supported(supportedLayers)) {
        throw std::runtime_error("Required layers not supported!");
    }

    auto constexpr appInfo = vk::ApplicationInfo(
            "HelloTriangle",
            1,
            "vkTut",
            1,
            VK_API_VERSION_1_2);

    auto const instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &appInfo,
            supportedLayers.size(),
            supportedLayers.data(),
            requiredExtensions.size(),
            requiredExtensions.data());

    return vk::createInstanceUnique(instanceCreateInfo);
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
