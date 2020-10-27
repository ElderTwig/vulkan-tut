#include "glfwUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <gsl/gsl>

#include <string>
#include <memory>
#include <vector>

namespace {
struct InitAndTerminateGLFW {
    InitAndTerminateGLFW()
    {
        glfwInit();
    }

    InitAndTerminateGLFW(InitAndTerminateGLFW&&)      = delete;
    InitAndTerminateGLFW(InitAndTerminateGLFW const&) = delete;
    auto
    operator=(InitAndTerminateGLFW&&) = delete;
    auto
    operator=(InitAndTerminateGLFW const&) = delete;

    ~InitAndTerminateGLFW()
    {
        glfwTerminate();
    }
} const terminateGLFW{};
}    // namespace

namespace glfwUtils {

auto
create_window(
        int const width,
        int const height,
        bool resizeable,
        std::string const& title) -> UniqueWindow
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, (int)resizeable);

    return UniqueWindow{
            glfwCreateWindow(width, height, title.data(), nullptr, nullptr)};
}

auto
required_vk_extensions() -> std::vector<char const*>
{
    auto numExtensions = uint32_t{};
    auto const* const extensionsCharArray =
            glfwGetRequiredInstanceExtensions(&numExtensions);

    auto const extensionsSpan =
            gsl::make_span(extensionsCharArray, numExtensions);

    return std::vector<char const*>{
            std::cbegin(extensionsSpan),
            std::cend(extensionsSpan)};
}

}    // namespace glfwUtils
