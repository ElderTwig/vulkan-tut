#include "vulkanUtility.hpp"
#include "shaderUtility.hpp"

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

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT /*unused*/,
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
        vk::SurfaceKHR const& surface) -> QueueFamilyAndPos
{
    auto presentationFamily = firstGraphicsQueueFamily;

    try {
        while(device.getSurfaceSupportKHR(presentationFamily.position, surface)
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
swap_chain_unique(
        vk::SurfaceKHR const& surface,
        vk::SurfaceCapabilitiesKHR const& surfaceCaps,
        vk::SurfaceFormatKHR const& surfaceFormat,
        vk::PresentModeKHR const& presentationMode,
        vk::Extent2D const currentDimensions,
        std::vector<uint32_t> const& queueFamilyIndicies,
        vk::UniqueDevice const& logicalDevice) -> vk::UniqueSwapchainKHR
{
    auto const sharingMode = queueFamilyIndicies.size() > 1u
                                     ? vk::SharingMode::eConcurrent
                                     : vk::SharingMode::eExclusive;

    return logicalDevice->createSwapchainKHRUnique(vk::SwapchainCreateInfoKHR(
            {},
            surface,
            surfaceCaps.minImageCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            currentDimensions,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            sharingMode,
            queueFamilyIndicies.size(),
            queueFamilyIndicies.data(),
            surfaceCaps.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentationMode,
            VK_TRUE));
}

[[nodiscard]] auto
create_swap_chain(
        vk::SurfaceKHR const& surface,
        vk::SurfaceCapabilitiesKHR const& surfaceCaps,
        vk::SurfaceFormatKHR const& surfaceFormat,
        vk::PresentModeKHR const& presentationMode,
        vk::UniqueDevice const& logicalDevice,
        vk::Extent2D requestedDimensions,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR
{
    return swap_chain_unique(
            surface,
            surfaceCaps,
            surfaceFormat,
            presentationMode,
            clamp_extent_dimensions(
                    surfaceCaps.minImageExtent,
                    surfaceCaps.maxImageExtent,
                    requestedDimensions),
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
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
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
            0,
            nullptr,
            1,
            &colourAttachmentReference);

    auto constexpr subpassDependency = vk::SubpassDependency(
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits(0),
            vk::AccessFlagBits::eColorAttachmentWrite);

    auto const renderPassCreationInfo = vk::RenderPassCreateInfo(
            {},
            1,
            &colourAttachment,
            1,
            &subpass,
            1,
            &subpassDependency);

    return logicalDevice->createRenderPassUnique(renderPassCreationInfo);
}

[[nodiscard]] auto
graphics_pipeline_create_info(
        vk::PipelineCreateFlags const flags,
        ShaderStageInfoVec const* shaderStages,
        VertexInputState const* vertexInputState,
        InputAssemblyState const* inputAssemblyState,
        TessellationState const* tessellationState,
        ViewportState const* viewportState,
        RasterizationState const* rasterizationState,
        MultisampleState const* multisampleState,
        DepthStencilState const* depthStencilState,
        ColourBlendState const* colourBlendState,
        DynamicState const* dynamicState,
        vk::UniquePipelineLayout const& layout,
        vk::UniqueRenderPass const& renderPass,
        uint32_t const renderPassPosition) -> vk::GraphicsPipelineCreateInfo
{
    return vk::GraphicsPipelineCreateInfo(
            flags,
            shaderStages != nullptr ? shaderStages->size() : 0,
            shaderStages != nullptr ? shaderStages->data() : nullptr,
            vertexInputState,
            inputAssemblyState,
            tessellationState,
            viewportState,
            rasterizationState,
            multisampleState,
            depthStencilState,
            colourBlendState,
            dynamicState,
            *layout,
            *renderPass,
            renderPassPosition,
            nullptr,
            -1);
}

[[nodiscard]] auto
create_graphics_pipeline(
        vk::UniqueDevice const& logicalDevice,
        shaderUtils::VertexShader const& vertexShader,
        shaderUtils::FragmentShader const& fragmentShader,
        int const width,
        int const height,
        ColourBlendState const& colourBlendState,
        DynamicState const* dynamicState,
        vk::UniquePipelineLayout const& pipelineLayout,
        vk::UniqueRenderPass const& renderPass) -> vk::UniquePipeline
{
    auto const shaderStageInfos = ShaderStageInfoVec{
            shaderUtils::shader_stage_creation_info(
                    vertexShader.module,
                    shaderUtils::VertexShader::type,
                    "main"),
            shaderUtils::shader_stage_creation_info(
                    fragmentShader.module,
                    shaderUtils::FragmentShader::type,
                    "main")};

    auto constexpr vertexInput = VertexInputState{};

    auto constexpr inputAssembly = InputAssemblyState(
            {},
            vk::PrimitiveTopology::eTriangleList,
            vkFalse);

    auto constexpr* tessellationStatePtr =
            static_cast<TessellationState*>(nullptr);

    auto const viewport      = create_viewport(width, height);
    auto const scissor       = create_scissor(width, height);
    auto const viewportState = create_viewport_state(viewport, scissor);

    auto const rasterisationState = RasterizationState{
            {},
            vkFalse,
            vkFalse,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack,
            vk::FrontFace::eClockwise,
            vkFalse,
            0.0f,
            0.0f,
            0.0f,
            1.0f};

    auto constexpr multisampleState  = MultisampleState{};
    auto constexpr depthStencilState = DepthStencilState{};

    return logicalDevice
            ->createGraphicsPipelineUnique(
                    nullptr,
                    vulkanUtils::graphics_pipeline_create_info(
                            {},
                            &shaderStageInfos,
                            &vertexInput,
                            &inputAssembly,
                            tessellationStatePtr,
                            &viewportState,
                            &rasterisationState,
                            &multisampleState,
                            &depthStencilState,
                            &colourBlendState,
                            dynamicState,
                            pipelineLayout,
                            renderPass,
                            0))
            .value;
}

[[nodiscard]] auto
create_framebuffer(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueRenderPass const& renderPass,
        vk::UniqueImageView const& imageView,
        vk::Extent2D const imageDimensions) -> vk::UniqueFramebuffer
{
    auto const creationInfo = vk::FramebufferCreateInfo(
            {},
            *renderPass,
            1,
            &imageView.get(),
            imageDimensions.width,
            imageDimensions.height,
            1);

    return logicalDevice->createFramebufferUnique(creationInfo);
}

[[nodiscard]] auto
create_framebuffers(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueRenderPass const& renderPass,
        std::vector<vk::UniqueImageView> const& imageViews,
        vk::Extent2D imageDimensions) -> std::vector<vk::UniqueFramebuffer>
{
    auto framebuffers = std::vector<vk::UniqueFramebuffer>(imageViews.size());
    std::transform(
            std::cbegin(imageViews),
            std::cend(imageViews),
            std::begin(framebuffers),
            [&](vk::UniqueImageView const& imageView) {
                return create_framebuffer(
                        logicalDevice,
                        renderPass,
                        imageView,
                        imageDimensions);
            });

    return framebuffers;
}

[[nodiscard]] auto
create_command_pool(
        vk::UniqueDevice const& logicalDevice,
        QueueFamilyAndPos const& queueFamily) -> vk::UniqueCommandPool
{
    auto const creationInfo =
            vk::CommandPoolCreateInfo({}, queueFamily.position);

    return logicalDevice->createCommandPoolUnique(creationInfo);
}

[[nodiscard]] auto
allocate_command_buffers(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueCommandPool const& commandPool,
        vk::CommandBufferLevel const commandLevel,
        uint32_t const nrBuffers) -> std::vector<vk::UniqueCommandBuffer>
{
    auto const allocationInfo = vk::CommandBufferAllocateInfo(
            *commandPool,
            commandLevel,
            nrBuffers);

    return logicalDevice->allocateCommandBuffersUnique(allocationInfo);
}

}    // namespace vulkanUtils
