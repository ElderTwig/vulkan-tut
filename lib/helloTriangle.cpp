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
        vulkanUtils::QueueFamily const& graphics,
        vulkanUtils::QueueFamily const& presentation) -> std::vector<uint32_t>
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
            m_extensions{[] {
                auto glfwExtensions = glfwUtils::required_vk_extensions();
                glfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                return glfwExtensions;
            }()},
            m_instance{m_validationLayers, m_extensions},
            m_dynamicFuncDispatcher{*m_instance},
            m_debugMessenger{*m_instance, *m_dynamicFuncDispatcher},
            m_physicalDevice{*m_instance, m_deviceExtensions},
            m_surface{*m_instance, *m_physicalDevice, m_window.get()},
            m_surfaceCaps{
                    m_physicalDevice->getSurfaceCapabilitiesKHR(*m_surface)},
            m_surfaceFormats{
                    m_physicalDevice->getSurfaceFormatsKHR(*m_surface)},
            m_graphicsQueues{m_physicalDevice.graphics_queue_family()},
            m_presentationQueues{m_physicalDevice.present_queue_family(
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
        vk::Device const& logicalDevice,
        vk::Semaphore const& imageAvailableSignal,
        vk::Semaphore const& renderFinishedSignal,
        vk::Fence const& memFence,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue,
        vk::SwapchainKHR const& swapChain,
        std::vector<vk::UniqueCommandBuffer> const& commandBuffers) -> void
{
    auto const signaled = logicalDevice.waitForFences(
            memFence,
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

    if(signaled != vk::Result::eSuccess) {
        throw std::runtime_error("Fence could not be signaled\n");
    }

    try {
        auto const nextImageIndex =
                logicalDevice
                        .acquireNextImageKHR(
                                swapChain,
                                std::numeric_limits<uint64_t>::max(),
                                imageAvailableSignal,
                                nullptr)
                        .value;

        auto constexpr pipelineStage = std::array{
                (vk::PipelineStageFlags)
                        vk::PipelineStageFlagBits::eColorAttachmentOutput};
        auto const submitInfo = vk::SubmitInfo(
                1,
                &imageAvailableSignal,
                pipelineStage.data(),
                1,
                &commandBuffers[nextImageIndex].get(),
                1,
                &renderFinishedSignal);

        logicalDevice.resetFences(memFence);
        graphicsQueue.submit(submitInfo, memFence);

        auto const presentInfo = vk::PresentInfoKHR(
                1,
                &renderFinishedSignal,
                1,
                &swapChain,
                &nextImageIndex,
                nullptr);

        if(presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
            std::cerr << "Frame could not be presented!\n";
        }
    }
    catch(vk::OutOfDateKHRError const&) {
        // recreate_swap_chain(
        // surfaceSize(physicalDevice, surface),
        // physicalDevice,
        // logicalDevice,
        // surface,
        // queueIndicies,
        // fragShader,
        // vertShader,
        // colourBlendState,
        // pipelineLayout,
        // commandPool,
        // swapChain,
        // swapChainImages,
        // imageViews,
        // renderPass,
        // graphicsPipeline,
        // framebuffers,
        // commandBuffers);
    }
}

auto
draw_frames(
        vk::Device const& logicalDevice,
        SemaphoreArray const& imageAvailableSignals,
        SemaphoreArray const& renderFinishedSignals,
        MemoryFenceArray const& memFences,
        vk::Queue const& graphicsQueue,
        vk::Queue const& presentQueue,
        vk::SwapchainKHR const& swapChain,
        std::vector<vk::UniqueCommandBuffer> const& commandBuffers) -> void
{
    for(auto i = 0u; i < maxFramesInFlight; ++i) {
        draw_frame(
                logicalDevice,
                imageAvailableSignals[i].get(),
                renderFinishedSignals[i].get(),
                memFences[i].get(),
                graphicsQueue,
                presentQueue,
                swapChain,
                commandBuffers);
    }
}

auto
HelloTriangle::main_loop() -> void
{
    while(glfwWindowShouldClose(m_window.get()) == 0) {
        glfwPollEvents();
        draw_frames(
                m_logicalDevice.get(),
                m_imageAvailableSignals,
                m_renderFinishedSignals,
                m_memoryFences,
                m_graphicsQueue,
                m_presentQueue,
                m_swapChain.get(),
                m_commandBuffers);
    }

    m_logicalDevice->waitIdle();
}

auto
HelloTriangle::cleanup() -> void
{
    glfwTerminate();
}
