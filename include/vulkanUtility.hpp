#ifndef VK_TUT_VULKAN_UTILITY
#define VK_TUT_VULKAN_UTILITY

#include "glfwUtility.hpp"
#include "shaderUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>

namespace vulkanUtils {

auto constexpr vkFalse = VK_FALSE;
auto constexpr vkTrue  = VK_TRUE;

struct SwapChainDetails {
    vk::SurfaceCapabilitiesKHR surfaceCaps;
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentationMode;
};

struct LoaderDispatcherPair {
    vk::DynamicLoader loader;
    vk::DispatchLoaderDynamic dispatcher;
};

struct QueueFamilyAndPos {
    vk::QueueFamilyProperties properties;
    long position;
};

struct QueueFamilies {
    QueueFamilyAndPos graphics;
    QueueFamilyAndPos present;
};

[[nodiscard]] auto
extensions_supported(std::vector<char const*> const& extensionNames) noexcept
        -> bool;

[[nodiscard]] auto
layers_supported(std::vector<char const*> const& layerNames) noexcept -> bool;

[[nodiscard]] auto
create_instance(std::vector<char const*> const& requiredValidationLayers)
        -> vk::UniqueInstance;

[[nodiscard]] auto
create_loader_dispatcher_pair(vk::UniqueInstance const& instance)
        -> LoaderDispatcherPair;

using UniqueDebugUtilsMessengerEXT =
        vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;

[[nodiscard]] auto
create_debug_messenger(
        vk::UniqueInstance const& instance,
        vk::DispatchLoaderDynamic const& dispatcher)
        -> UniqueDebugUtilsMessengerEXT;

[[nodiscard]] auto
get_graphics_queues(vk::PhysicalDevice const& device) -> QueueFamilyAndPos;

[[nodiscard]] auto
get_present_queues(
        vk::PhysicalDevice const& device,
        QueueFamilyAndPos firstGraphicsQueueFamily,
        vk::UniqueSurfaceKHR const& surface) -> QueueFamilyAndPos;

[[nodiscard]] auto
create_logical_device(
        vk::PhysicalDevice const& physicalDevice,
        QueueFamilyAndPos const& queue,
        std::vector<float> const& queuePriorities,
        std::vector<char const*> const& validationLayers,
        std::vector<char const*> const& extensions) -> vk::UniqueDevice;

[[nodiscard]] auto
best_device(
        vk::UniqueInstance const& instance,
        std::vector<char const*> requiredExtensions) -> vk::PhysicalDevice;

auto constexpr defaultSurfaceFormat = vk::SurfaceFormatKHR{
        vk::Format::eB8G8R8A8Srgb,
        vk::ColorSpaceKHR::eSrgbNonlinear};

auto constexpr defaultPresentationMode = vk::PresentModeKHR::eFifoRelaxed;

[[nodiscard]] auto
choose_static_swap_chain_details(
        vk::PhysicalDevice const& device,
        vk::UniqueSurfaceKHR const& surface,
        std::vector<vk::SurfaceFormatKHR> const& requestedFormats,
        std::vector<vk::PresentModeKHR> const& requestedPresentModes)
        -> SwapChainDetails;

[[nodiscard]] auto
create_swap_chain(
        vk::UniqueSurfaceKHR const& surface,
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueDevice const& logicalDevice,
        SwapChainDetails const& staticCreationDetails,
        vk::Extent2D requestedDimensions,
        std::vector<uint32_t> const& queueFamilyIndicies)
        -> vk::UniqueSwapchainKHR;

[[nodiscard]] auto
create_image_views(
        vk::UniqueDevice const& logicalDevice,
        std::vector<vk::Image> const& images,
        vk::Format imageFormat) -> std::vector<vk::UniqueImageView>;

[[nodiscard]] auto constexpr create_viewport(int width, int height) noexcept
        -> vk::Viewport
{
    return vk::Viewport(0.f, 0.f, (float)width, (float)height, 0.f, 1.f);
}

[[nodiscard]] auto constexpr create_scissor(int width, int height) noexcept
        -> vk::Rect2D
{
    return vk::Rect2D({}, vk::Extent2D(width, height));
}

[[nodiscard]] auto constexpr create_viewport_state(
        vk::Viewport const& viewport,
        vk::Rect2D const& scissor) noexcept
        -> vk::PipelineViewportStateCreateInfo
{
    return vk::PipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);
}

auto constexpr allColourComponents =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
        | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

auto constexpr defaultBlendAttachment = vk::PipelineColorBlendAttachmentState(
        vkTrue,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eSrcAlpha,
        vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd,
        allColourComponents);

auto constexpr defaultBlendConstants = std::array{1.0f, 1.0f, 1.0f, 1.0f};

auto constexpr defaultBlendState = vk::PipelineColorBlendStateCreateInfo(
        {},
        vkFalse,
        {},
        1,
        &defaultBlendAttachment,
        defaultBlendConstants);

auto constexpr defaultDynamicStates =
        std::array{vk::DynamicState::eViewport, vk::DynamicState::eLineWidth};

auto constexpr defaultDynamicStateInfo = vk::PipelineDynamicStateCreateInfo(
        {},
        defaultDynamicStates.size(),
        defaultDynamicStates.data());

[[nodiscard]] auto
create_render_pass(vk::UniqueDevice const& logicalDevice, vk::Format format)
        -> vk::UniqueRenderPass;

using ShaderStageInfoVec = std::vector<vk::PipelineShaderStageCreateInfo>;
using VertexInputState   = vk::PipelineVertexInputStateCreateInfo;
using InputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo;
using TessellationState  = vk::PipelineTessellationStateCreateInfo;
using ViewportState      = vk::PipelineViewportStateCreateInfo;
using RasterizationState = vk::PipelineRasterizationStateCreateInfo;
using MultisampleState   = vk::PipelineMultisampleStateCreateInfo;
using DepthStencilState  = vk::PipelineDepthStencilStateCreateInfo;
using ColourBlendState   = vk::PipelineColorBlendStateCreateInfo;
using DynamicState       = vk::PipelineDynamicStateCreateInfo;

[[nodiscard]] auto
graphics_pipeline_create_info(
        vk::PipelineCreateFlags flags,
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
        uint32_t renderPassPosition) -> vk::GraphicsPipelineCreateInfo;

[[nodiscard]] auto
create_graphics_pipeline(
        vk::UniqueDevice const& logicalDevice,
        shaderUtils::VertexShader const& vertexShader,
        shaderUtils::FragmentShader const& fragmentShader,
        int width,
        int height,
        ColourBlendState const& colourBlendState,
        DynamicState const* dynamicState,
        vk::UniquePipelineLayout const& pipelineLayout,
        vk::UniqueRenderPass const& renderPass) -> vk::UniquePipeline;

[[nodiscard]] auto
create_framebuffers(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueRenderPass const& renderPass,
        std::vector<vk::UniqueImageView> const& imageViews,
        vk::Extent2D imageDimensions) -> std::vector<vk::UniqueFramebuffer>;

[[nodiscard]] auto
create_command_pool(
        vk::UniqueDevice const& logicalDevice,
        QueueFamilyAndPos const& queueFamily) -> vk::UniqueCommandPool;

[[nodiscard]] auto
allocate_command_buffers(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueCommandPool const& commandPool,
        vk::CommandBufferLevel commandLevel,
        uint32_t nrBuffers) -> std::vector<vk::UniqueCommandBuffer>;

}    // namespace vulkanUtils

#endif    // VK_TUT_VULKAN_UTILITY
