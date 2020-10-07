#include "helloTriangle.hpp"

#include <gsl/gsl>

#include <algorithm>
#include <string>
#include <vulkan/vulkan_core.h>

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
getSupportedExtensions() -> std::vector<std::string>
{
    auto const supportedExtensionProperties =
            vk::enumerateInstanceExtensionProperties();

    auto supportedExtensions =
            std::vector<std::string>(supportedExtensionProperties.size());

    std::transform(
            supportedExtensionProperties.cbegin(),
            supportedExtensionProperties.cend(),
            supportedExtensions.begin(),
            [](auto&& property) -> std::string {
                return property.extensionName;
            });

    return supportedExtensions;
}

auto
checkExtensionSupport(std::vector<std::string> const& requiredExtensions)
        -> void
{
    auto const supportedExtensions = getSupportedExtensions();
    for(auto const& extension : requiredExtensions) {
        auto const iterator = std::find(
                supportedExtensions.cbegin(),
                supportedExtensions.cend(),
                extension);

        if(iterator == supportedExtensions.cend()) {
            throw std::runtime_error(extension + " not supported!");
        }
    }
}

auto
getRequiredExtensions() -> std::vector<char const*>
{
    auto extensionCount = uint32_t{};

    auto const** glfwExtentions =
            glfwGetRequiredInstanceExtensions(&extensionCount);

    auto const span = gsl::make_span(glfwExtentions, extensionCount);

    auto vec = std::vector<char const*>{std::cbegin(span), std::cend(span)};
    vec.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return vec;
}

auto
getSupportedLayers() -> std::vector<std::string>
{
    auto const supportedLayerProps = vk::enumerateInstanceLayerProperties();
    auto supportedLayers = std::vector<std::string>(supportedLayerProps.size());

    std::transform(
            std::cbegin(supportedLayerProps),
            std::cend(supportedLayerProps),
            std::begin(supportedLayers),
            [](auto&& property) -> std::string { return property.layerName; });

    return supportedLayers;
}

auto
checkLayerSupport(std::vector<std::string> const& requiredLayers) -> void
{
    auto const supportedLayers = getSupportedLayers();

    for(auto const& layer : requiredLayers) {
        auto const iterator = std::find(
                std::cbegin(supportedLayers),
                std::cend(supportedLayers),
                layer);

        if(iterator == std::cend(supportedLayers)) {
            throw std::runtime_error(layer + " layer is not supported!");
        }
    }
}

auto
HelloTriangle::createInstace() -> vk::Instance
{
    auto const requiredExtensions = getRequiredExtensions();

    checkExtensionSupport(std::vector<std::string>{
            std::cbegin(requiredExtensions),
            std::cend(requiredExtensions)});

    auto constexpr layers = std::array{"VK_LAYER_KHRONOS_validation"};
    checkLayerSupport(
            std::vector<std::string>{std::cbegin(layers), std::cend(layers)});

    auto constexpr appInfo = vk::ApplicationInfo(
            "HelloTriangle",
            1,
            "vkTut",
            1,
            VK_API_VERSION_1_2);

    auto const instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &appInfo,
            layers.size(),
            layers.data(),
            requiredExtensions.size(),
            requiredExtensions.data());

    auto instance = vk::Instance{};
    auto const result =
            vk::createInstance(&instanceCreateInfo, nullptr, &instance);

    if(result != vk::Result::eSuccess) {
        throw std::runtime_error("Vk instance could not be created");
    }

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
