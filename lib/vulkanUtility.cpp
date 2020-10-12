#include "vulkanUtility.hpp"

#include <numeric>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstring>
#include <vector>
#include <iostream>
#include <string>

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

[[nodiscard]] auto
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

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/)
{
    if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << pCallbackData->pMessage << '\n';
    }

    return (VkBool32) false;
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

[[nodiscard]] auto
best_device(
        vk::UniqueInstance const& instance,
        std::vector<char const*> requiredExtensions) -> vk::PhysicalDevice
{
    auto constexpr getExtensionName = [](vk::ExtensionProperties const& prop) {
        return prop.extensionName;
    };
    auto constexpr getSupportedExtensions =
            [getExtensionName](vk::PhysicalDevice const& device) {
                auto const props = device.enumerateDeviceExtensionProperties();
                auto names       = std::vector<char const*>(props.size());
                std::transform(
                        std::cbegin(props),
                        std::cend(props),
                        std::begin(names),
                        getExtensionName);

                std::sort(std::begin(names), std::end(names));
                return names;
            };

    std::sort(std::begin(requiredExtensions), std::end(requiredExtensions));

    auto const unsuitable = [&](vk::PhysicalDevice const& device) {
        auto const supportedExtensions         = getSupportedExtensions(device);
        auto const requiredExtensionsSupported = std::includes(
                std::begin(supportedExtensions),
                std::cend(supportedExtensions),
                std::cbegin(requiredExtensions),
                std::cend(requiredExtensions));

        auto const hasGeometryShaderSuport =
                device.getFeatures().geometryShader == 0u;

        return hasGeometryShaderSuport && requiredExtensionsSupported;
    };

    auto devices = instance->enumeratePhysicalDevices();
    devices.erase(
            std::remove_if(std::begin(devices), std::end(devices), unsuitable),
            std::end(devices));

    if(devices.empty()) {
        throw std::runtime_error("No suitable devices detected!");
    }

    auto constexpr score = [](vk::PhysicalDevice const& device) {
        return 1000
               * static_cast<int>(
                       device.getProperties().deviceType
                       == vk::PhysicalDeviceType::eDiscreteGpu);
    };

    std::nth_element(
            std::begin(devices),
            std::begin(devices),
            std::end(devices),
            [score](auto&& lhs, auto&& rhs) {
                return score(lhs) > score(rhs);
            });

    return devices[0];
}

[[nodiscard]] auto
get_next_graphics_queue_family(
        vk::PhysicalDevice const& device,
        long startIndex) -> QueueFamilyAndPos
{
    auto constexpr isGraphicsQueue = [](vk::QueueFamilyProperties const& prop) {
        return (prop.queueFlags & vk::QueueFlagBits::eGraphics)
               == vk::QueueFlagBits::eGraphics;
    };

    auto const queueProperties = device.getQueueFamilyProperties();
    auto const queueIterator   = std::find_if(
            std::cbegin(queueProperties) + startIndex,
            std::cend(queueProperties),
            isGraphicsQueue);

    if(queueIterator == std::cend(queueProperties)) {
        throw std::runtime_error{"Physical device has no next graphics queue!"};
    }

    return {*queueIterator, queueIterator - std::cbegin(queueProperties)};
}

[[nodiscard]] auto
get_graphics_queues(vk::PhysicalDevice const& device) -> QueueFamilyAndPos
{
    return get_next_graphics_queue_family(device, 0);
}

[[nodiscard]] auto
get_present_queues(
        vk::PhysicalDevice const& device,
        QueueFamilyAndPos firstGraphicsQueueFamily,
        vk::UniqueSurfaceKHR const& surface) -> QueueFamilyAndPos
{
    auto presentationFamily = firstGraphicsQueueFamily;

    try {
        while(device.getSurfaceSupportKHR(
                      presentationFamily.position,
                      surface.get())
              == 0u) {
            presentationFamily = get_next_graphics_queue_family(
                    device,
                    presentationFamily.position + 1);
        }
    }
    catch(std::exception const& e) {
        using namespace std::string_literals;
        throw std::runtime_error(
                "Could not find a presentationQueueFamily because:\n"s
                + e.what());
    }

    return presentationFamily;
}

[[nodiscard]] auto
create_logical_device(
        vk::PhysicalDevice const& physicalDevice,
        QueueFamilyAndPos const& queue,
        std::vector<float> const& queuePriorities,
        std::vector<char const*> const& extensions) -> vk::UniqueDevice
{
    auto const queueCreationInfo = vk::DeviceQueueCreateInfo(
            {},
            queue.position,
            queue.properties.queueCount,
            queuePriorities.data());

    auto const deviceFeatures = physicalDevice.getFeatures();

    auto const deviceCreationInfo = vk::DeviceCreateInfo(
            {},
            queueCreationInfo.queueCount,
            &queueCreationInfo,
            {},
            {},
            extensions.size(),
            extensions.data(),
            &deviceFeatures);

    return physicalDevice.createDeviceUnique(deviceCreationInfo);
}

}    // namespace vulkanUtils
