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

Surface::Surface(vk::Instance const& instance, GLFWwindow* window) :
            m_surface{create_window_surface(instance, window)},
            m_boundInstance{instance},
            m_boundWindow{window}
{}

[[nodiscard]] auto
Surface::boundInstance() const noexcept -> vk::Instance const&
{
    return m_boundInstance.get();
}

[[nodiscard]] auto
Surface::boundWindow() const noexcept -> GLFWwindow const*
{
    return m_boundWindow;
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
