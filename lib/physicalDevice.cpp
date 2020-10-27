#include "physicalDevice.hpp"

namespace vulkanUtils {

PhysicalDevice::PhysicalDevice(
        vk::Instance const& instance,
        std::vector<char const*> extensions) :
            m_physicalDevice{},
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
