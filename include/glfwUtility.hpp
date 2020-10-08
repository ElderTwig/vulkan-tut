#ifndef VK_TUT_GLFW_UTILITY
#define VK_TUT_GLFW_UTILITY

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <string>
#include <memory>
#include <vector>

namespace glfwUtils {
struct WindowDeleter {
    auto
    operator()(GLFWwindow* window) noexcept -> void
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

using UniqueWindow = std::unique_ptr<GLFWwindow, WindowDeleter>;

auto
create_window(int width, int height, bool resizeable, std::string const& title)
        -> UniqueWindow;

auto
required_vk_extensions() -> std::vector<char const*>;
}    // namespace glfwUtils

#endif    // VK_TUT_GLFW_UTILITY
