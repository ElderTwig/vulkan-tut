#include "physicalDevice.hpp"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <functional>
#include <cstring>
#include <tuple>

[[nodiscard]] auto
get_next_graphics_queue_family(
        vk::PhysicalDevice const& device,
        long startIndex) -> vulkanUtils::QueueFamily
{
    auto constexpr isGraphicsQueue = [](vk::QueueFamilyProperties const& prop) {
        return (prop.queueFlags & vk::QueueFlagBits::eGraphics)
               == vk::QueueFlagBits::eGraphics;
    };

    auto const queueProperties = device.getQueueFamilyProperties();
    auto const queueIterator   = std::find_if(
            std::cbegin(queueProperties) + startIndex,
            std::cend(queueProperties),
            isGraphicsQueue);

    if(queueIterator == std::cend(queueProperties)) {
        throw std::runtime_error{"Physical device has no next graphics queue!"};
    }

    return {*queueIterator, queueIterator - std::cbegin(queueProperties)};
}

[[nodiscard]] auto
supports_extensions(
        vk::PhysicalDevice const& device,
        std::vector<char const*> const& requiredExtensions) noexcept -> bool
{
    auto const supportedExtensions =
            device.enumerateDeviceExtensionProperties();

    for(auto const& requiredExtension : requiredExtensions) {
        bool supportsExtension = false;
        for(auto const& supportedExtension : supportedExtensions) {
            if(strcmp(supportedExtension.extensionName, requiredExtension)
               != 0) {
                supportsExtension = true;
            }
        }

        if(!supportsExtension) {
            std::cerr << requiredExtension << " not supported.\n";
            return false;
        }
    }

    return true;
}

[[nodiscard]] auto
get_compatible_device(
        vk::Instance const& instance,
        std::vector<char const*> const& requiredExtensions) noexcept
        -> vk::PhysicalDevice
{
    auto const physicalDevices = instance.enumeratePhysicalDevices();
    return *std::find_if(
            physicalDevices.cbegin(),
            physicalDevices.cend(),
            [&](auto&& device) {
                return supports_extensions(device, requiredExtensions)
                       && device.getFeatures().geometryShader;
            });
}

namespace vulkanUtils {
PhysicalDevice::PhysicalDevice(
        vk::Instance const& instance,
        std::vector<char const*> extensions) :
            m_physicalDevice{get_compatible_device(instance, extensions)},
            m_extensions{std::move(extensions)},
            m_boundInstance{instance}
{}

[[nodiscard]] auto
PhysicalDevice::boundInstance() const noexcept -> vk::Instance const&
{
    return m_boundInstance.get();
}

[[nodiscard]] auto
PhysicalDevice::extensions() const noexcept -> std::vector<char const*> const&
{
    return m_extensions;
}

[[nodiscard]] auto
PhysicalDevice::operator*() const noexcept -> vk::PhysicalDevice const&
{
    return m_physicalDevice;
}

[[nodiscard]] auto
PhysicalDevice::operator->() const noexcept -> vk::PhysicalDevice const*
{
    return &m_physicalDevice;
}

[[nodiscard]] auto
PhysicalDevice::graphics_queue_family() const -> QueueFamily
{
    return get_next_graphics_queue_family(m_physicalDevice, 0);
}

[[nodiscard]] auto
PhysicalDevice::present_queue_family(
        QueueFamily firstGraphicsQueueFamily,
        vk::SurfaceKHR const& surface) const -> QueueFamily
{
    auto presentationFamily = firstGraphicsQueueFamily;

    try {
        while(m_physicalDevice.getSurfaceSupportKHR(
                      presentationFamily.position,
                      surface)
              == 0u) {
            presentationFamily = get_next_graphics_queue_family(
                    m_physicalDevice,
                    presentationFamily.position + 1);
        }
    }
    catch(std::exception const& e) {
        using namespace std::string_literals;
        throw std::runtime_error(
                "Could not find a presentationQueueFamily because:\n"s
                + e.what());
    }

    return presentationFamily;
}

}    // namespace vulkanUtils
