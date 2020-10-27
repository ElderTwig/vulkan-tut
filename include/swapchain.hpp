#ifndef VK_TUT_SWAPCHAIN_HPP
#define VK_TUT_SWAPCHAIN_HPP

#include <vulkan/vulkan.hpp>

#include <vector>

namespace vulkanUtils {
class Swapchain {
    explicit Swapchain(
            vk::Device const& logicalDevice,
            vk::SurfaceKHR const& surface);
};
}    // namespace vulkanUtils

#endif    // VK_TUT_SWAPCHAIN_HPP
