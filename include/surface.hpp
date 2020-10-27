#ifndef VK_TUT_SURFACE
#define VK_TUT_SURFACE

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <functional>

namespace vulkanUtils {

class Surface {
public:
    explicit Surface(vk::Instance const& instance, GLFWwindow* window);

    [[nodiscard]] auto
    boundInstance() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    boundWindow() const noexcept -> GLFWwindow const*;

    [[nodiscard]] auto
    operator*() const noexcept -> vk::SurfaceKHR const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::SurfaceKHR const*;

private:
    vk::UniqueSurfaceKHR const m_surface;

    std::reference_wrapper<vk::Instance const> const m_boundInstance;
    GLFWwindow const* m_boundWindow;
};

}    // namespace vulkanUtils

#endif    // VK_TUT_SURFACE
