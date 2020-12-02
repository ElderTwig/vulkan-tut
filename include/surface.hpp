#ifndef VK_TUT_SURFACE
#define VK_TUT_SURFACE

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <functional>

namespace vulkanUtils {

[[nodiscard]] auto
surfaceFormatSupported(
        std::vector<vk::SurfaceFormatKHR> const& supportedFormats,
        vk::SurfaceFormatKHR const& format) noexcept -> bool;

[[nodiscard]] auto
firstSupportedSurfaceFormat(
        std::vector<vk::SurfaceFormatKHR> const& supportedFormats,
        std::vector<vk::SurfaceFormatKHR> const& requestedFormats) noexcept
        -> vk::SurfaceFormatKHR;

[[nodiscard]] auto
surfacePresentationModeSupported(
        std::vector<vk::PresentModeKHR> const& supportedPresentationModes,
        vk::PresentModeKHR const& presentationMode) noexcept -> bool;

[[nodiscard]] auto
firstSupportedPresentationMode(
        std::vector<vk::PresentModeKHR> const& supportedPresentationModes,
        std::vector<vk::PresentModeKHR> const&
                requestedPresentationModes) noexcept -> vk::PresentModeKHR;
}    // namespace vulkanUtils

namespace vulkanUtils {

class Surface {
public:
    explicit Surface(
            vk::Instance const& instance,
            vk::PhysicalDevice const& physicalDevice,
            GLFWwindow* window);

    [[nodiscard]] auto
    boundInstance() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    boundPhysicalDevice() const noexcept -> vk::PhysicalDevice const&;

    [[nodiscard]] auto
    boundWindow() const noexcept -> GLFWwindow const*;

    [[nodiscard]] auto
    capabilities() const noexcept -> vk::SurfaceCapabilitiesKHR const&;

    [[nodiscard]] auto
    formats() const noexcept -> std::vector<vk::SurfaceFormatKHR> const&;

    [[nodiscard]] auto
    presentationModes() const noexcept
            -> std::vector<vk::PresentModeKHR> const&;

    [[nodiscard]] auto
    operator*() const noexcept -> vk::SurfaceKHR const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::SurfaceKHR const*;

private:
    vk::UniqueSurfaceKHR const m_surface;

    vk::SurfaceCapabilitiesKHR const m_surfaceCaps;
    std::vector<vk::SurfaceFormatKHR> const m_surfaceFormats;
    std::vector<vk::PresentModeKHR> const m_surfacePresentationModes;

    std::reference_wrapper<vk::Instance const> const m_boundInstance;
    std::reference_wrapper<vk::PhysicalDevice const> m_boundPhysicalDevice;
    GLFWwindow const* m_boundWindow;
};

}    // namespace vulkanUtils

#endif    // VK_TUT_SURFACE
