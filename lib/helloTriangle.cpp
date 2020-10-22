#include "helloTriangle.hpp"

#include "glfwUtility.hpp"
#include "vulkanUtility.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <gsl/gsl>

#include <algorithm>
#include <string>

[[nodiscard]] auto
create_index_list(
        vulkanUtils::QueueFamilyAndPos const& graphics,
        vulkanUtils::QueueFamilyAndPos const& presentation)
        -> std::vector<uint32_t>
{
    auto const duplicateQueues = graphics.position == presentation.position;

    if(duplicateQueues) {
        return {};
    }

    return {static_cast<unsigned int>(graphics.position),
            static_cast<unsigned int>(presentation.position)};
}

[[nodiscard]] auto
record_commands(
        vk::UniqueRenderPass const& renderPass,
        vk::UniqueFramebuffer const& framebuffer,
        vk::Extent2D dimensions,
        vk::UniquePipeline const& pipeline,
        vk::UniqueCommandBuffer& commandBuffer)
{
    auto constexpr beginInfo = vk::CommandBufferBeginInfo{};

    commandBuffer->begin(beginInfo);

    auto const clearColour = vk::ClearValue{{{std::array{0.f, 0.f, 0.f, 1.f}}}};

    auto const renderPassBeginInfo = vk::RenderPassBeginInfo(
            *renderPass,
            *framebuffer,
            vk::Rect2D{{0, 0}, dimensions},
            1,
            &clearColour);

    commandBuffer->beginRenderPass(
            renderPassBeginInfo,
            vk::SubpassContents::eInline);

    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

    commandBuffer->draw(3, 1, 0, 0);

    commandBuffer->endRenderPass();
    commandBuffer->end();
}

[[nodiscard]] auto
record_commands_to_command_buffers(
        vk::UniqueRenderPass const& renderPass,
        std::vector<vk::UniqueFramebuffer> const& framebuffers,
        vk::Extent2D dimensions,
        vk::UniquePipeline const& pipeline,
        std::vector<vk::UniqueCommandBuffer>&& commandBuffers)
        -> std::vector<vk::UniqueCommandBuffer>
{
    if(framebuffers.size() != commandBuffers.size()) {
        throw std::runtime_error(
                "Framebuffer and commandBuffer vectors are not the same size!");
    }

    for(auto i = 0u; i < framebuffers.size(); ++i) {
        record_commands(
                renderPass,
                framebuffers[i],
                dimensions,
                pipeline,
                commandBuffers[i]);
    }

    return std::move(commandBuffers);
}

HelloTriangle::HelloTriangle() :
            m_window{glfwUtils::create_window(width, height, false, "test")},
            m_instance{vulkanUtils::create_instance(m_validationLayers)},
            m_loaderDispatcherPair{
                    vulkanUtils::create_loader_dispatcher_pair(m_instance)},
            m_debugMessenger{vulkanUtils::create_debug_messenger(
                    m_instance,
                    m_loaderDispatcherPair.dispatcher)},
            m_surface{glfwUtils::create_window_surface(m_instance, m_window)},
            m_physicalDevice{
                    vulkanUtils::best_device(m_instance, m_deviceExtensions)},
            m_graphicsQueues{
                    vulkanUtils::get_graphics_queues(m_physicalDevice)},
            m_presentationQueues{vulkanUtils::get_present_queues(
                    m_physicalDevice,
                    m_graphicsQueues,
                    m_surface)},
            m_queuePriorities(m_graphicsQueues.properties.queueCount, 1.0f),
            m_queueIndicies{
                    create_index_list(m_graphicsQueues, m_presentationQueues)},
            m_logicalDevice{vulkanUtils::create_logical_device(
                    m_physicalDevice,
                    m_graphicsQueues,
                    m_queuePriorities,
                    m_validationLayers,
                    m_deviceExtensions)},
            m_graphicsQueue{
                    m_logicalDevice->getQueue(m_graphicsQueues.position, 0)},
            m_presentQueue{m_logicalDevice->getQueue(
                    m_presentationQueues.position,
                    0)},
            m_staticSwapChainDetails{
                    vulkanUtils::choose_static_swap_chain_details(
                            m_physicalDevice,
                            m_surface,
                            m_requestedSurfaceFormats,
                            m_requestedPresentationModes)},
            m_swapChain{vulkanUtils::create_swap_chain(
                    m_surface,
                    m_logicalDevice,
                    m_staticSwapChainDetails,
                    {width, height},
                    m_queueIndicies)},
            m_swapChainImages(
                    m_logicalDevice->getSwapchainImagesKHR(*m_swapChain)),
            m_imageViews{vulkanUtils::create_image_views(
                    m_logicalDevice,
                    m_swapChainImages,
                    m_staticSwapChainDetails.format.format)},
            m_vertShader{m_logicalDevice, "triangle"},
            m_fragShader{m_logicalDevice, "triangle"},
            m_pipelineCreationInfos{
                    shaderUtils::shader_stage_creation_info(
                            m_vertShader.module,
                            shaderUtils::VertexShader::type,
                            "main"),
                    shaderUtils::shader_stage_creation_info(
                            m_fragShader.module,
                            shaderUtils::FragmentShader::type,
                            "main")},
            m_vertexInputInfo{},
            m_inputAssembly(
                    {},
                    vk::PrimitiveTopology::eTriangleList,
                    vulkanUtils::vkFalse),
            m_viewPort(vulkanUtils::create_viewport(width, height)),
            m_scissor(vulkanUtils::create_scissor(width, height)),
            m_viewportState{
                    vulkanUtils::create_viewport_state(m_viewPort, m_scissor)},
            m_rasterizerState(
                    {},
                    {},
                    {},
                    {},
                    vk::CullModeFlagBits::eBack,
                    vk::FrontFace::eClockwise),
            m_multisamplingState{},
            m_depthStencilState{},
            m_colourBlendAttatchment{vulkanUtils::defaultBlendAttachment},
            m_colourBlendState{vulkanUtils::defaultBlendState},
            m_pipelineLayout{m_logicalDevice->createPipelineLayoutUnique(
                    vk::PipelineLayoutCreateInfo{})},
            m_renderpass{vulkanUtils::create_render_pass(
                    m_logicalDevice,
                    m_staticSwapChainDetails.format.format)},
            m_graphicsPipeline{
                    m_logicalDevice
                            ->createGraphicsPipelineUnique(
                                    {},
                                    vulkanUtils::graphics_pipeline_create_info(
                                            {},
                                            &m_pipelineCreationInfos,
                                            &m_vertexInputInfo,
                                            &m_inputAssembly,
                                            nullptr,
                                            &m_viewportState,
                                            &m_rasterizerState,
                                            &m_multisamplingState,
                                            &m_depthStencilState,
                                            &m_colourBlendState,
                                            nullptr,
                                            m_pipelineLayout,
                                            m_renderpass,
                                            0))
                            .value},
            m_framebuffers{vulkanUtils::create_framebuffers(
                    m_logicalDevice,
                    m_renderpass,
                    m_imageViews,
                    {width, height})},
            m_commandPool{vulkanUtils::create_command_pool(
                    m_logicalDevice,
                    m_graphicsQueues)},
            m_commandBuffers{record_commands_to_command_buffers(
                    m_renderpass,
                    m_framebuffers,
                    {width, height},
                    m_graphicsPipeline,
                    vulkanUtils::allocate_command_buffers(
                            m_logicalDevice,
                            m_commandPool,
                            vk::CommandBufferLevel::ePrimary,
                            m_framebuffers.size()))},
            m_imageAvailableSignals{
                    m_logicalDevice->createSemaphoreUnique({}),
                    m_logicalDevice->createSemaphoreUnique({})},
            m_renderFinishedSignals{
                    m_logicalDevice->createSemaphoreUnique({}),
                    m_logicalDevice->createSemaphoreUnique({})},
            m_memoryFences{
                    m_logicalDevice->createFenceUnique(
                            {vk::FenceCreateFlagBits::eSignaled}),
                    m_logicalDevice->createFenceUnique(
                            {vk::FenceCreateFlagBits::eSignaled})}
{
    std::cerr << m_physicalDevice.getProperties().deviceName.data() << '\n';
}

//[[nodiscard]] auto
// HelloTriangle::recreate_swap_chain(vk::Extent2D const newDimensions)
//{
// auto const swapChain [[maybe_unused]] = vulkanUtils::create_swap_chain(
// m_physicalDevice,
// m_surface,
// m_logicalDevice,
// newDimensions,
//{m_surfaceFormat},
// m_presentationModes,
// m_queueIndicies);

// auto const imageViews = vulkanUtils::create_image_views(
// m_logicalDevice,
// m_swapChainImages,
//);
//}

auto
draw_frame(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueSwapchainKHR const& swapChain,
        vk::UniqueSemaphore const& imageAvailableSignal,
        vk::UniqueSemaphore const& renderFinishedSignal,
        vk::UniqueFence const& memFence,
        std::vector<vk::UniqueCommandBuffer> const& commandBuffers,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue) -> void
{
    auto const signaled = logicalDevice->waitForFences(
            *memFence,
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

    if(signaled != vk::Result::eSuccess) {
        throw std::runtime_error("Fence could not be signaled\n");
    }

    logicalDevice->resetFences(*memFence);

    auto const nextImageIndex =
            logicalDevice
                    ->acquireNextImageKHR(
                            *swapChain,
                            std::numeric_limits<uint64_t>::max(),
                            *imageAvailableSignal,
                            nullptr)
                    .value;

    auto constexpr pipelineStage = std::array{
            (vk::PipelineStageFlags)
                    vk::PipelineStageFlagBits::eColorAttachmentOutput};
    auto const submitInfo = vk::SubmitInfo(
            1,
            &imageAvailableSignal.get(),
            pipelineStage.data(),
            1,
            &commandBuffers[nextImageIndex].get(),
            1,
            &renderFinishedSignal.get());

    graphicsQueue.submit(submitInfo, *memFence);

    auto const presentInfo = vk::PresentInfoKHR(
            1,
            &renderFinishedSignal.get(),
            1,
            &swapChain.get(),
            &nextImageIndex,
            nullptr);

    if(presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
        std::cerr << "Frame could not be presented!\n";
    }
}

auto
draw_frames(
        vk::UniqueDevice const& logicalDevice,
        vk::UniqueSwapchainKHR const& swapChain,
        SemaphoreArray const& imageAvailableSignals,
        SemaphoreArray const& renderFinishedSignals,
        MemoryFenceArray const& memFences,
        std::vector<vk::UniqueCommandBuffer> const& commandBuffers,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue) -> void
{
    for(auto i = 0u; i < maxFramesInFlight; ++i) {
        draw_frame(
                logicalDevice,
                swapChain,
                imageAvailableSignals[i],
                renderFinishedSignals[i],
                memFences[i],
                commandBuffers,
                graphicsQueue,
                presentQueue);
    }
}

auto
HelloTriangle::main_loop() -> void
{
    while(glfwWindowShouldClose(m_window.get()) == 0) {
        glfwPollEvents();
        draw_frames(
                m_logicalDevice,
                m_swapChain,
                m_imageAvailableSignals,
                m_renderFinishedSignals,
                m_memoryFences,
                m_commandBuffers,
                m_graphicsQueue,
                m_presentQueue);
    }

    m_logicalDevice->waitIdle();
}

auto
HelloTriangle::cleanup() -> void
{
    glfwTerminate();
}
