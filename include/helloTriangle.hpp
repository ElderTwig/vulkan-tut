#ifndef VK_TUT_HELLO_TRIANGLE_HPP
#define VK_TUT_HELLO_TRIANGLE_HPP

#include "glfwUtility.hpp"
#include "surface.hpp"
#include "vulkanUtility.hpp"
#include "shaderUtility.hpp"
#include "instance.hpp"
#include "loaderDispatcher.hpp"
#include "debugMessenger.hpp"
#include "physicalDevice.hpp"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>

static int constexpr maxFramesInFlight{2};
using SemaphoreArray   = std::array<vk::UniqueSemaphore, maxFramesInFlight>;
using MemoryFenceArray = std::array<vk::UniqueFence, maxFramesInFlight>;

class HelloTriangle {
public:
    static int constexpr width  = 800;
    static int constexpr height = 600;

    HelloTriangle();

    auto
    run() -> void
    {
        main_loop();
        cleanup();
    }

private:
    glfwUtils::UniqueWindow const m_window;

    std::vector<char const*> const m_validationLayers{
            "VK_LAYER_KHRONOS_validation"};
    std::vector<char const*> const m_extensions;

    vulkanUtils::Instance const m_instance;

    vulkanUtils::DynamicFuncDispatcher const m_dynamicFuncDispatcher;
    vulkanUtils::DebugMessenger const m_debugMessenger;
    vulkanUtils::Surface const m_surface;

    std::vector<char const*> const m_deviceExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vulkanUtils::PhysicalDevice const m_physicalDevice;

    vulkanUtils::QueueFamilyAndPos const m_graphicsQueues;
    vulkanUtils::QueueFamilyAndPos const m_presentationQueues;
    std::vector<float> const m_queuePriorities;
    std::vector<uint32_t> const m_queueIndicies;

    vk::UniqueDevice const m_logicalDevice;
    vk::Queue const m_graphicsQueue;
    vk::Queue const m_presentQueue;

    std::vector<vk::SurfaceFormatKHR> const m_requestedSurfaceFormats{
            vulkanUtils::defaultSurfaceFormat};
    std::vector<vk::PresentModeKHR> const m_requestedPresentationModes{
            vk::PresentModeKHR::eFifoRelaxed,
            vk::PresentModeKHR::eFifo};

    vulkanUtils::SwapChainDetails const m_staticSwapChainDetails;

    shaderUtils::VertexShader const m_vertShader;
    shaderUtils::FragmentShader const m_fragShader;

    vk::PipelineColorBlendAttachmentState const m_colourBlendAttatchment;
    vk::PipelineColorBlendStateCreateInfo const m_colourBlendState;

    vk::UniquePipelineLayout const m_pipelineLayout;
    vk::UniqueCommandPool const m_commandPool;

    SemaphoreArray const m_imageAvailableSignals;
    SemaphoreArray const m_renderFinishedSignals;
    MemoryFenceArray const m_memoryFences;

    vk::UniqueSwapchainKHR m_swapChain;
    std::vector<vk::Image> m_swapChainImages;
    std::vector<vk::UniqueImageView> m_imageViews;

    vk::UniqueRenderPass m_renderPass;
    vk::UniquePipeline m_graphicsPipeline;

    std::vector<vk::UniqueFramebuffer> m_framebuffers;
    std::vector<vk::UniqueCommandBuffer> m_commandBuffers;

    auto
    main_loop() -> void;

    static auto
    cleanup() -> void;
};

#endif    // VK_TUT_HELLO_TRIANGLE_HPP
