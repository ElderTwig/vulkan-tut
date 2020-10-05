#include "helloTriangle.hpp"
#include <algorithm>

auto
HelloTriangle::initWindow() noexcept -> UniqueGLFWwindow
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return UniqueGLFWwindow{
            glfwCreateWindow(width, height, "vk", nullptr, nullptr)};
}

auto
extensionsSupported(char const** extensions, int nrExtentions) -> void
{
    auto const supportedExtensions = vk::enumerateInstanceExtensionProperties();

    std::for_each(
            extensions,
            extensions + nrExtentions,
            [&](auto const* extension) {
                auto const found = std::cend(supportedExtensions)
                                   != std::find(
                                           std::cbegin(supportedExtensions),
                                           std::cend(supportedExtensions),
                                           extension);
                if(!found) {
                    std::cerr << "Extension " << extension
                              << " not supported!\n";
                }
            });
}

auto
HelloTriangle::createInstace() -> vk::Instance
{
    auto extensionCount = uint32_t{};

    auto const** glfwExtentions =
            glfwGetRequiredInstanceExtensions(&extensionCount);

    auto constexpr appInfo = vk::ApplicationInfo(
            "HelloTriangle",
            1,
            "vkTut",
            1,
            VK_API_VERSION_1_2);

    auto const instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &appInfo,
            {},
            {},
            extensionCount,
            glfwExtentions);

    auto instance = vk::Instance{};
    auto const result =
            vk::createInstance(&instanceCreateInfo, nullptr, &instance);

    if(result != vk::Result::eSuccess) {
        throw std::runtime_error("Vk instance could not be created");
    }

    extensionsSupported(glfwExtentions, extensionCount);

    return instance;
}

auto
HelloTriangle::initVulkan() -> void
{}

auto
HelloTriangle::mainLoop() -> void
{
    while(glfwWindowShouldClose(m_window.get()) == 0) {
        glfwPollEvents();
    }
}

auto
HelloTriangle::cleanup() -> void
{
    glfwTerminate();
}
