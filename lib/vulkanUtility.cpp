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
create_instance(std::vector<char const*> const& requiredValidationLayers)
        -> vk::UniqueInstance
{
    auto const requiredExtensions = [] {
        auto extensions = glfwUtils::required_vk_extensions();
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return extensions;
    }();

    if(!extensions_supported(requiredExtensions)) {
        throw std::runtime_error("Required extensions not supported!");
    }

    if(!layers_supported(requiredValidationLayers)) {
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
            requiredValidationLayers.size(),
            requiredValidationLayers.data(),
            requiredExtensions.size(),
            requiredExtensions.data());

    return vk::createInstanceUnique(instanceCreateInfo);
}

[[nodiscard]] auto
create_loader_dispatcher_pair(vk::UniqueInstance const& instance)
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
        vk::UniqueInstance const& instance,
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
        std::vector<char const*> const& validationLayers,
        std::vector<char const*> const& extensions) -> vk::UniqueDevice
{
    auto const queueCreationInfo = vk::DeviceQueueCreateInfo(
            {},
            queue.position,
            1,
            queuePriorities.data());

    static auto const deviceFeatures = physicalDevice.getFeatures();

    auto const deviceCreationInfo = vk::DeviceCreateInfo(
            {},
            queueCreationInfo.queueCount,
            &queueCreationInfo,
            validationLayers.size(),
            validationLayers.data(),
            extensions.size(),
            extensions.data(),
            &deviceFeatures);

    return physicalDevice.createDeviceUnique(deviceCreationInfo);
}

struct SwapChainDetails {
    vk::SurfaceCapabilitiesKHR surfaceCaps;
    vk::Extent2D dimensions;
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentationMode;
};

[[nodiscard]] auto
clamp_extent_dimensions(
        vk::Extent2D const minExtent,
        vk::Extent2D const maxExtent,
        vk::Extent2D const requestedExtent) -> vk::Extent2D
{
    auto const maxAndMinAreOrdered =
            std::tie(maxExtent.width, maxExtent.height)
            >= std::tie(minExtent.width, minExtent.height);

    if(!(maxAndMinAreOrdered)) {
        throw std::logic_error(
                "maxExtent is not greater than or equal to minExtent");
    }

    auto const clampedWidth =
            std::clamp(requestedExtent.width, minExtent.width, maxExtent.width);

    auto const clampedHeight = std::clamp(
            requestedExtent.height,
            minExtent.height,
            maxExtent.height);

    return {clampedWidth, clampedHeight};
}

[[nodiscard]] auto
choose_format(
        std::vector<vk::SurfaceFormatKHR> const availableFormats,
        std::vector<vk::SurfaceFormatKHR> const& requestedSurfaceFormats)
        -> vk::SurfaceFormatKHR
{
    auto const requestedFormatSupported =
            [&](vk::SurfaceFormatKHR const& requestedSurfaceFormat) {
                return std::find(
                               std::cbegin(availableFormats),
                               std::cend(availableFormats),
                               requestedSurfaceFormat)
                       != std::cend(availableFormats);
            };

    for(auto const& requested : requestedSurfaceFormats) {
        if(requestedFormatSupported(requested)) {
            return requested;
        }
    }

    std::cerr << "Requested surface formats not supported\n";

    auto const defaultFormatNotSupported =
            !requestedFormatSupported(defaultSurfaceFormat);
    if(defaultFormatNotSupported) {
        throw std::runtime_error(
                "Default SurfaceFormatKHR not supported!\n"
                "Please request a supported format.");
    }

    return defaultSurfaceFormat;
}
[[nodiscard]] auto
choose_presentation_mode(
        std::vector<vk::PresentModeKHR> const supportedModes,
        std::vector<vk::PresentModeKHR> const& requestedPresentModes)
        -> vk::PresentModeKHR
{
    auto const requestedModeSupported =
            [&](vk::PresentModeKHR const& requestedPresentMode) {
                return std::find(
                               std::cbegin(supportedModes),
                               std::cend(supportedModes),
                               requestedPresentMode)
                       != std::cend(supportedModes);
            };

    for(auto const& requested : requestedPresentModes) {
        if(requestedModeSupported(requested)) {
            return requested;
        }
    }

    std::cerr << "Requested presentation modes are not supported!\n";

    auto const defaultModeNotSupported =
            !requestedModeSupported(defaultPresentationMode);
    if(defaultModeNotSupported) {
        throw std::runtime_error(
                "Default PresentModeKHR is not supported!\n"
                "Please request a supported PresentModeKHR");
    }

    return defaultPresentationMode;
}

[[nodiscard]] auto
choose_swap_chain_details(
        vk::PhysicalDevice const& device,
        vk::UniqueSurfaceKHR const& surface,
        vk::Extent2D const requestedExtent,
        std::vector<vk::SurfaceFormatKHR> const& requestedFormats,
        std::vector<vk::PresentModeKHR> const& requestedPresentModes)
        -> SwapChainDetails
{
    auto const surfaceCaps   = device.getSurfaceCapabilitiesKHR(*surface);
    auto const clampedExtent = clamp_extent_dimensions(
            surfaceCaps.minImageExtent,
            surfaceCaps.maxImageExtent,
            requestedExtent);

    auto const chosenFormat = choose_format(
            device.getSurfaceFormatsKHR(*surface),
            requestedFormats);

    auto const chosenPresentMode = choose_presentation_mode(
            device.getSurfacePresentModesKHR(*surface),
            requestedPresentModes);

    return {surfaceCaps, clampedExtent, chosenFormat, chosenPresentMode};
}

[[nodiscard]] auto
swap_chain_unique(
        SwapChainDetails const&& creationDetails,
        vk::UniqueSurfaceKHR const& surface,
        std::vector<uint32_t> const& queueFamilyIndicies,
        vk::UniqueDevice const& logicalDevice) -> vk::UniqueSwapchainKHR
{
    auto&& [surfaceCaps, dimensions, format, presentMode] = creationDetails;

    auto const sharingMode = queueFamilyIndicies.size() > 1u
                                     ? vk::SharingMode::eConcurrent
                                     : vk::SharingMode::eExclusive;

    return logicalDevice->createSwapchainKHRUnique(vk::SwapchainCreateInfoKHR(
            {},
            *surface,
            surfaceCaps.minImageCount,
            format.format,
            format.colorSpace,
            dimensions,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            sharingMode,
            queueFamilyIndicies.size(),
            queueFamilyIndicies.data(),
            surfaceCaps.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentMode,
            VK_TRUE));
}

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D const requestedDimensions,
        vk::SurfaceFormatKHR const requestedFormat,
        vk::PresentModeKHR const requestedPresentMode,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR
{
    return swap_chain_unique(
            choose_swap_chain_details(
                    physicalDevice,
                    surface,
                    requestedDimensions,
                    {requestedFormat},
                    {requestedPresentMode}),
            surface,
            queueFamilyIndicies,
            logicalDevice);
}    // namespace vulkanUtils

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR
{
    return swap_chain_unique(
            choose_swap_chain_details(
                    physicalDevice,
                    surface,
                    requestedDimensions,
                    {defaultSurfaceFormat},
                    {defaultPresentationMode}),
            surface,
            queueFamilyIndicies,
            logicalDevice);
}

[[nodiscard]] auto
create_swap_chain(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueSurfaceKHR const& surface,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        std::vector<vk::SurfaceFormatKHR> requestedFormats,
        std::vector<vk::PresentModeKHR> requestedPresentModes,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR
{
    return swap_chain_unique(
            choose_swap_chain_details(
                    physicalDevice,
                    surface,
                    requestedDimensions,
                    requestedFormats,
                    requestedPresentModes),
            surface,
            queueFamilyIndicies,
            logicalDevice);
}

[[nodiscard]] auto
create_image_view(
        vk::UniqueDevice const& logicalDevice,
        vk::Image const& image,
        vk::Format const imageFormat) -> vk::UniqueImageView
{
    auto constexpr subresourceRange = vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1);

    auto const creationInfo = vk::ImageViewCreateInfo(
            {},
            image,
            vk::ImageViewType::e2D,
            imageFormat,
            {},
            subresourceRange);

    return logicalDevice->createImageViewUnique(creationInfo);
}

[[nodiscard]] auto
create_image_views(
        vk::UniqueDevice const& logicalDevice,
        std::vector<vk::Image> const& images,
        vk::Format const imageFormat) -> std::vector<vk::UniqueImageView>
{
    auto imageViews = std::vector<vk::UniqueImageView>(images.size());

    std::transform(
            std::cbegin(images),
            std::cend(images),
            std::begin(imageViews),
            [&](vk::Image const& image) {
                return create_image_view(logicalDevice, image, imageFormat);
            });

    return imageViews;
}

[[nodiscard]] auto constexpr colour_attachment(vk::Format format) noexcept
        -> vk::AttachmentDescription
{
    return vk::AttachmentDescription(
            {},
            format,
            vk::SampleCountFlagBits::e1,
            {},
            {},
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            static_cast<vk::ImageLayout>(vk::AttachmentLoadOp::eDontCare),
            vk::ImageLayout::ePresentSrcKHR);
}

[[nodiscard]] auto
create_render_pass(vk::UniqueDevice const& logicalDevice, vk::Format format)
        -> vk::UniqueRenderPass
{
    auto const colourAttachment          = colour_attachment(format);
    auto const colourAttachmentReference = vk::AttachmentReference{
            0,
            vk::ImageLayout::eColorAttachmentOptimal};

    auto const subpass = vk::SubpassDescription(
            {},
            vk::PipelineBindPoint::eGraphics,
            {},
            {},
            1,
            &colourAttachmentReference);

    auto const renderPassCreationInfo =
            vk::RenderPassCreateInfo({}, 1, &colourAttachment, 1, &subpass);

    return logicalDevice->createRenderPassUnique(renderPassCreationInfo);
}

}    // namespace vulkanUtils
