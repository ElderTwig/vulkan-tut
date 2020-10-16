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
create_instance(std::vector<char const*> const& requiredValidationLayers)
        -> vk::UniqueInstance;

struct LoaderDispatcherPair {
    vk::DynamicLoader loader;
    vk::DispatchLoaderDynamic dispatcher;
};

[[nodiscard]] auto
create_loader_dispatcher_pair(vk::UniqueInstance const& instance)
        -> LoaderDispatcherPair;

using UniqueDebugUtilsMessengerEXT =
        vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;

[[nodiscard]] auto
create_debug_messenger(
        vk::UniqueInstance const& instance,
        vk::DispatchLoaderDynamic const& dispatcher)
        -> UniqueDebugUtilsMessengerEXT;

struct QueueFamilyAndPos {
    vk::QueueFamilyProperties properties;
    long position;
};

struct QueueFamilies {
    QueueFamilyAndPos graphics;
    QueueFamilyAndPos present;
};

[[nodiscard]] auto
get_graphics_queues(vk::PhysicalDevice const& device) -> QueueFamilyAndPos;

[[nodiscard]] auto
get_present_queues(
        vk::PhysicalDevice const& device,
        QueueFamilyAndPos firstGraphicsQueueFamily,
        vk::UniqueSurfaceKHR const& surface) -> QueueFamilyAndPos;

[[nodiscard]] auto
create_logical_device(
        vk::PhysicalDevice const& physicalDevice,
        QueueFamilyAndPos const& queue,
        std::vector<float> const& queuePriorities,
        std::vector<char const*> const& validationLayers,
        std::vector<char const*> const& extensions) -> vk::UniqueDevice;

[[nodiscard]] auto
best_device(
        vk::UniqueInstance const& instance,
        std::vector<char const*> requiredExtensions) -> vk::PhysicalDevice;

auto constexpr defaultSurfaceFormat = vk::SurfaceFormatKHR{
        vk::Format::eB8G8R8A8Srgb,
        vk::ColorSpaceKHR::eSrgbNonlinear};

auto constexpr defaultPresentationMode = vk::PresentModeKHR::eFifoRelaxed;

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        vk::SurfaceFormatKHR requestedFormat,
        vk::PresentModeKHR requestedPresentMode,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR;

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR;

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        std::vector<vk::SurfaceFormatKHR> requestedFormats,
        std::vector<vk::PresentModeKHR> requestedPresentModes,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR;

[[nodiscard]] auto
create_image_views(
        vk::UniqueDevice const& logicalDevice,
        std::vector<vk::Image> const& images,
        vk::Format imageFormat) -> std::vector<vk::UniqueImageView>;

}    // namespace vulkanUtils

#endif    // VK_TUT_VULKAN_UTILITY
