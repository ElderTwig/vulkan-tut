#include "physicalDevice.hpp"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <functional>
#include <cstring>

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

}    // namespace vulkanUtils
