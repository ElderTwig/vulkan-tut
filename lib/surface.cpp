#include "surface.hpp"

[[nodiscard]] auto
create_window_surface(vk::Instance const& instance, GLFWwindow* window)
        -> vk::UniqueSurfaceKHR
{
    auto* surface = VkSurfaceKHR{};
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface)
       != VK_SUCCESS) {
        throw std::runtime_error("Surface could not be created!");
    }

    return vk::UniqueSurfaceKHR{surface, instance};
}

namespace vulkanUtils {

[[nodiscard]] auto
surfaceFormatSupported(
        std::vector<vk::SurfaceFormatKHR> const& supportedFormats,
        vk::SurfaceFormatKHR const& format) noexcept -> bool
{
    return std::find(
                   std::begin(supportedFormats),
                   std::end(supportedFormats),
                   format)
           != std::end(supportedFormats);
}

[[nodiscard]] auto
surfacePresentationModeSupported(
        std::vector<vk::PresentModeKHR> const& supportedPresentationModes,
        vk::PresentModeKHR const& presentationMode) noexcept -> bool
{
    return std::find(
                   std::begin(supportedPresentationModes),
                   std::end(supportedPresentationModes),
                   presentationMode)
           != std::end(supportedPresentationModes);
}

}    // namespace vulkanUtils

namespace vulkanUtils {

Surface::Surface(
        vk::Instance const& instance,
        vk::PhysicalDevice const& physicalDevice,
        GLFWwindow* window) :
            m_surface{create_window_surface(instance, window)},
            m_surfaceCaps{physicalDevice.getSurfaceCapabilitiesKHR(*m_surface)},
            m_surfaceFormats{physicalDevice.getSurfaceFormatsKHR(*m_surface)},
            m_surfacePresentationModes{
                    physicalDevice.getSurfacePresentModesKHR(*m_surface)},
            m_boundInstance{instance},
            m_boundPhysicalDevice{physicalDevice},
            m_boundWindow{window}
{}

[[nodiscard]] auto
Surface::boundInstance() const noexcept -> vk::Instance const&
{
    return m_boundInstance.get();
}

[[nodiscard]] auto
Surface::boundPhysicalDevice() const noexcept -> vk::PhysicalDevice const&
{
    return m_boundPhysicalDevice.get();
}

[[nodiscard]] auto
Surface::boundWindow() const noexcept -> GLFWwindow const*
{
    return m_boundWindow;
}

[[nodiscard]] auto
Surface::capabilities() const noexcept -> vk::SurfaceCapabilitiesKHR const&
{
    return m_surfaceCaps;
}

[[nodiscard]] auto
Surface::formats() const noexcept -> std::vector<vk::SurfaceFormatKHR> const&
{
    return m_surfaceFormats;
}

[[nodiscard]] auto
Surface::presentationModes() const noexcept
        -> std::vector<vk::PresentModeKHR> const&
{
    return m_surfacePresentationModes;
}

[[nodiscard]] auto
Surface::operator*() const noexcept -> vk::SurfaceKHR const&
{
    return *m_surface;
}

[[nodiscard]] auto
Surface::operator->() const noexcept -> vk::SurfaceKHR const*
{
    return &m_surface.get();
}

}    // namespace vulkanUtils
