#include "vulkanUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstring>
#include <vector>
#include <iostream>

namespace vulkanUtils {

[[nodiscard]] auto
extensions_supported(std::vector<char const*> const& extensionNames) noexcept
        -> bool
{
    auto const supportedProperties = vk::enumerateInstanceExtensionProperties();

    auto allSupported = true;
    for(auto const* name : extensionNames) {
        auto const iterator = std::find_if(
                std::cbegin(supportedProperties),
                std::cend(supportedProperties),
                [name](auto&& property) {
                    return strcmp(name, property.extensionName.data()) == 0;
                });

        if(iterator == std::cend(supportedProperties)) {
            std::cerr << "Extension " << iterator->extensionName.data()
                      << " not supported!\n";

            allSupported = false;
        }
    }

    return allSupported;
}

[[nodiscard]] auto
layers_supported(std::vector<char const*> const& layerNames) noexcept -> bool
{
    auto const supportedProperties = vk::enumerateInstanceLayerProperties();

    auto allSupported = true;
    for(auto const* name : layerNames) {
        auto const iterator = std::find_if(
                std::cbegin(supportedProperties),
                std::cend(supportedProperties),
                [name](auto&& property) {
                    return strcmp(name, property.layerName.data()) == 0;
                });

        if(iterator == std::cend(supportedProperties)) {
            std::cerr << "layer " << iterator->layerName.data()
                      << " not supported!\n";

            allSupported = false;
        }
    }

    return allSupported;
}

auto
create_instance() -> vk::UniqueInstance
{
    auto const requiredExtensions = [] {
        auto extensions = glfwUtils::required_vk_extensions();
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return extensions;
    }();

    if(!extensions_supported(requiredExtensions)) {
        throw std::runtime_error("Required extensions not supported!");
    }

    auto const requiredLayers = std::vector{"VK_LAYER_KHRONOS_validation"};

    if(!layers_supported(requiredLayers)) {
        throw std::runtime_error("Required layers not supported!");
    }

    auto constexpr appInfo = vk::ApplicationInfo(
            "HelloTriangle",
            1,
            "vkTut",
            1,
            VK_API_VERSION_1_2);

    auto const instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &appInfo,
            requiredLayers.size(),
            requiredLayers.data(),
            requiredExtensions.size(),
            requiredExtensions.data());

    return vk::createInstanceUnique(instanceCreateInfo);
}

[[nodiscard]] auto
create_loader_dispatcher_pair(vk::UniqueInstance& instance)
        -> LoaderDispatcherPair
{
    auto loader = vk::DynamicLoader{};
    auto const getInstanceProcAddress =
            loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
                    "vkGetInstanceProcAddr");
    auto dispatcher = vk::DispatchLoaderDynamic{getInstanceProcAddress};
    dispatcher.init(*instance);

    return {std::move(loader), std::move(dispatcher)};
}

[[nodiscard]] auto
create_debug_messenger(
        vk::UniqueInstance& instance,
        vk::DispatchLoaderDynamic const& dispatcher)
        -> UniqueDebugUtilsMessengerEXT
{
    using SeverityFlags          = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    auto constexpr severityFlags = SeverityFlags::eError | SeverityFlags::eInfo
                                   | SeverityFlags::eVerbose
                                   | SeverityFlags::eWarning;

    using TypeFlags          = vk::DebugUtilsMessageTypeFlagBitsEXT;
    auto constexpr typeFlags = TypeFlags::eGeneral | TypeFlags::ePerformance
                               | TypeFlags::eValidation;

    auto const creationInfo = vk::DebugUtilsMessengerCreateInfoEXT(
            {},
            severityFlags,
            typeFlags,
            debugCallback,
            nullptr);

    return instance->createDebugUtilsMessengerEXTUnique(
            creationInfo,
            nullptr,
            dispatcher);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
        VkDebugUtilsMessageTypeFlagsEXT,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/)
{
    // if(messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    // {
    std::cerr << pCallbackData->pMessage;
    //}

    return (vk::Bool32) false;
}

}    // namespace vulkanUtils
