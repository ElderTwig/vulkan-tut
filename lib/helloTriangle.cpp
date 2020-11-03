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

auto
recreate_swap_chain(
        vk::Extent2D const newDimensions,
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueDevice const& logicalDevice,
        vk::SurfaceKHR const& surface,
        vulkanUtils::SwapChainDetails const& staticSwapChainDetails,
        std::vector<uint32_t> const& queueIndicies,
        shaderUtils::FragmentShader const& fragShader,
        shaderUtils::VertexShader const& vertShader,
        vk::PipelineColorBlendStateCreateInfo const& colourBlendState,
        vk::UniquePipelineLayout const& pipelineLayout,
        vk::UniqueCommandPool const& commandPool,
        vk::UniqueSwapchainKHR& swapChain,
        std::vector<vk::Image>& swapChainImages,
        std::vector<vk::UniqueImageView>& imageViews,
        vk::UniqueRenderPass& renderPass,
        vk::UniquePipeline& graphicsPipeline,
        std::vector<vk::UniqueFramebuffer>& framebuffers,
        std::vector<vk::UniqueCommandBuffer>& commandBuffers) -> void
{
    logicalDevice->waitIdle();

    swapChain = vulkanUtils::create_swap_chain(
            surface,
            physicalDevice,
            logicalDevice,
            staticSwapChainDetails,
            newDimensions,
            queueIndicies);

    logicalDevice->getSwapchainImagesKHR(*swapChain).swap(swapChainImages);

    vulkanUtils::create_image_views(
            logicalDevice,
            swapChainImages,
            staticSwapChainDetails.format.format)
            .swap(imageViews);

    renderPass = vulkanUtils::create_render_pass(
            logicalDevice,
            staticSwapChainDetails.format.format);

    graphicsPipeline = vulkanUtils::create_graphics_pipeline(
            logicalDevice,
            vertShader,
            fragShader,
            newDimensions.width,
            newDimensions.height,
            colourBlendState,
            nullptr,
            pipelineLayout,
            renderPass);

    vulkanUtils::create_framebuffers(
            logicalDevice,
            renderPass,
            imageViews,
            newDimensions)
            .swap(framebuffers);

    record_commands_to_command_buffers(
            renderPass,
            framebuffers,
            newDimensions,
            graphicsPipeline,
            vulkanUtils::allocate_command_buffers(
                    logicalDevice,
                    commandPool,
                    vk::CommandBufferLevel::ePrimary,
                    framebuffers.size()))
            .swap(commandBuffers);
}

HelloTriangle::HelloTriangle() :
            m_window{glfwUtils::create_window(width, height, false, "test")},
            m_extensions{[] {
                auto glfwExtensions = glfwUtils::required_vk_extensions();
                glfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                return glfwExtensions;
            }()},
            m_instance{m_validationLayers, m_extensions},
            m_dynamicFuncDispatcher{*m_instance},
            m_debugMessenger{*m_instance, *m_dynamicFuncDispatcher},
            m_surface{*m_instance, m_window.get()},
            m_physicalDevice{*m_instance, m_deviceExtensions},
            m_graphicsQueues{
                    vulkanUtils::get_graphics_queues(*m_physicalDevice)},
            m_presentationQueues{vulkanUtils::get_present_queues(
                    *m_physicalDevice,
                    m_graphicsQueues,
                    *m_surface)},
            m_queuePriorities(m_graphicsQueues.properties.queueCount, 1.0f),
            m_queueIndicies{
                    create_index_list(m_graphicsQueues, m_presentationQueues)},
            m_logicalDevice{vulkanUtils::create_logical_device(
                    *m_physicalDevice,
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
                            *m_physicalDevice,
                            *m_surface,
                            m_requestedSurfaceFormats,
                            m_requestedPresentationModes)},
            m_vertShader{m_logicalDevice, "triangle"},
            m_fragShader{m_logicalDevice, "triangle"},
            m_colourBlendAttatchment{vulkanUtils::defaultBlendAttachment},
            m_colourBlendState{vulkanUtils::defaultBlendState},
            m_pipelineLayout{m_logicalDevice->createPipelineLayoutUnique(
                    vk::PipelineLayoutCreateInfo{})},
            m_commandPool{vulkanUtils::create_command_pool(
                    m_logicalDevice,
                    m_graphicsQueues)},
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
    recreate_swap_chain(
            {width, height},
            *m_physicalDevice,
            m_logicalDevice,
            *m_surface,
            m_staticSwapChainDetails,
            m_queueIndicies,
            m_fragShader,
            m_vertShader,
            m_colourBlendState,
            m_pipelineLayout,
            m_commandPool,
            m_swapChain,
            m_swapChainImages,
            m_imageViews,
            m_renderPass,
            m_graphicsPipeline,
            m_framebuffers,
            m_commandBuffers);
    std::cerr << m_physicalDevice->getProperties().deviceName.data() << '\n';
}

[[nodiscard]] auto
surfaceSize(
        vk::PhysicalDevice const& physicalDevice,
        vk::SurfaceKHR const& surface) -> vk::Extent2D
{
    return physicalDevice.getSurfaceCapabilitiesKHR(surface).currentExtent;
}

auto
draw_frame(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueDevice const& logicalDevice,
        vk::SurfaceKHR const& surface,
        vulkanUtils::SwapChainDetails const& staticSwapChainDetails,
        std::vector<uint32_t> const& queueIndicies,
        shaderUtils::FragmentShader const& fragShader,
        shaderUtils::VertexShader const& vertShader,
        vk::PipelineColorBlendStateCreateInfo const& colourBlendState,
        vk::UniquePipelineLayout const& pipelineLayout,
        vk::UniqueCommandPool const& commandPool,
        vk::UniqueSemaphore const& imageAvailableSignal,
        vk::UniqueSemaphore const& renderFinishedSignal,
        vk::UniqueFence const& memFence,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue,
        vk::UniqueSwapchainKHR& swapChain,
        std::vector<vk::Image>& swapChainImages,
        std::vector<vk::UniqueImageView>& imageViews,
        vk::UniqueRenderPass& renderPass,
        vk::UniquePipeline& graphicsPipeline,
        std::vector<vk::UniqueFramebuffer>& framebuffers,
        std::vector<vk::UniqueCommandBuffer>& commandBuffers) -> void
{
    auto const signaled = logicalDevice->waitForFences(
            *memFence,
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

    if(signaled != vk::Result::eSuccess) {
        throw std::runtime_error("Fence could not be signaled\n");
    }

    try {
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

        logicalDevice->resetFences(*memFence);
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
    catch(vk::OutOfDateKHRError const&) {
        recreate_swap_chain(
                surfaceSize(physicalDevice, surface),
                physicalDevice,
                logicalDevice,
                surface,
                staticSwapChainDetails,
                queueIndicies,
                fragShader,
                vertShader,
                colourBlendState,
                pipelineLayout,
                commandPool,
                swapChain,
                swapChainImages,
                imageViews,
                renderPass,
                graphicsPipeline,
                framebuffers,
                commandBuffers);
    }
}

auto
draw_frames(
        vk::PhysicalDevice const& physicalDevice,
        vk::UniqueDevice const& logicalDevice,
        vk::SurfaceKHR const& surface,
        vulkanUtils::SwapChainDetails const& staticSwapChainDetails,
        std::vector<uint32_t> const& queueIndicies,
        shaderUtils::FragmentShader const& fragShader,
        shaderUtils::VertexShader const& vertShader,
        vk::PipelineColorBlendStateCreateInfo const& colourBlendState,
        vk::UniquePipelineLayout const& pipelineLayout,
        vk::UniqueCommandPool const& commandPool,
        SemaphoreArray const& imageAvailableSignals,
        SemaphoreArray const& renderFinishedSignals,
        MemoryFenceArray const& memFences,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue,
        vk::UniqueSwapchainKHR& swapChain,
        std::vector<vk::Image>& swapChainImages,
        std::vector<vk::UniqueImageView>& imageViews,
        vk::UniqueRenderPass& renderPass,
        vk::UniquePipeline& graphicsPipeline,
        std::vector<vk::UniqueFramebuffer>& framebuffers,
        std::vector<vk::UniqueCommandBuffer>& commandBuffers) -> void
{
    for(auto i = 0u; i < maxFramesInFlight; ++i) {
        draw_frame(
                physicalDevice,
                logicalDevice,
                surface,
                staticSwapChainDetails,
                queueIndicies,
                fragShader,
                vertShader,
                colourBlendState,
                pipelineLayout,
                commandPool,
                imageAvailableSignals[i],
                renderFinishedSignals[i],
                memFences[i],
                graphicsQueue,
                presentQueue,
                swapChain,
                swapChainImages,
                imageViews,
                renderPass,
                graphicsPipeline,
                framebuffers,
                commandBuffers);
    }
}

auto
HelloTriangle::main_loop() -> void
{
    while(glfwWindowShouldClose(m_window.get()) == 0) {
        glfwPollEvents();
        draw_frames(
                *m_physicalDevice,
                m_logicalDevice,
                *m_surface,
                m_staticSwapChainDetails,
                m_queueIndicies,
                m_fragShader,
                m_vertShader,
                m_colourBlendState,
                m_pipelineLayout,
                m_commandPool,
                m_imageAvailableSignals,
                m_renderFinishedSignals,
                m_memoryFences,
                m_graphicsQueue,
                m_presentQueue,
                m_swapChain,
                m_swapChainImages,
                m_imageViews,
                m_renderPass,
                m_graphicsPipeline,
                m_framebuffers,
                m_commandBuffers);
    }

    m_logicalDevice->waitIdle();
}

auto
HelloTriangle::cleanup() -> void
{
    glfwTerminate();
}
