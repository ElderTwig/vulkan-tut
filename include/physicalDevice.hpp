#ifndef VK_TUT_PHYSICAL_DEVICE
#define VK_TUT_PHYSICAL_DEVICE

#include <vulkan/vulkan.hpp>

#include <vector>
#include <functional>

namespace vulkanUtils {

struct QueueFamily {
    vk::QueueFamilyProperties properties;
    long position;
};

class PhysicalDevice {
public:
    explicit PhysicalDevice(
            vk::Instance const& instance,
            std::vector<char const*> extensions);

    [[nodiscard]] auto
    boundInstance() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    extensions() const noexcept -> std::vector<char const*> const&;

    [[nodiscard]] auto
    operator*() const noexcept -> vk::PhysicalDevice const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::PhysicalDevice const*;

    [[nodiscard]] auto
    graphics_queue_family() const -> QueueFamily;

    [[nodiscard]] auto
    present_queue_family(
            QueueFamily firstGraphicsQueueFamily,
            vk::SurfaceKHR const& surface) const -> QueueFamily;

private:
    vk::PhysicalDevice const m_physicalDevice;
    std::vector<char const*> const m_extensions;

    std::reference_wrapper<vk::Instance const> const m_boundInstance;
};

}    // namespace vulkanUtils

#endif    // VK_TUT_PHYSICAL_DEVICE
