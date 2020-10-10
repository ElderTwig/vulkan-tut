#ifndef VK_TUT_VULKAN_UTILITY
#define VK_TUT_VULKAN_UTILITY

#include "glfwUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>

namespace vulkanUtils {

[[nodiscard]] auto
extensions_supported(std::vector<char const*> const& extensionNames) noexcept
        -> bool;

[[nodiscard]] auto
layers_supported(std::vector<char const*> const& layerNames) noexcept -> bool;

[[nodiscard]] auto
create_instance() -> vk::UniqueInstance;

struct LoaderDispatcherPair {
    vk::DynamicLoader loader;
    vk::DispatchLoaderDynamic dispatcher;
};

[[nodiscard]] auto
create_loader_dispatcher_pair(vk::UniqueInstance& instance)
        -> LoaderDispatcherPair;

using UniqueDebugUtilsMessengerEXT =
        vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;

[[nodiscard]] auto
create_debug_messenger(
        vk::UniqueInstance& instance,
        vk::DispatchLoaderDynamic const& dispatcher)
        -> UniqueDebugUtilsMessengerEXT;

[[nodiscard]] auto
best_device(vk::UniqueInstance const& instance) -> vk::PhysicalDevice;

[[nodiscard]] auto
create_logical_device(vk::PhysicalDevice const& physicalDevice)
        -> vk::UniqueDevice;

}    // namespace vulkanUtils

#endif    // VK_TUT_VULKAN_UTILITY
