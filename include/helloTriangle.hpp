#ifndef VK_TUT_HELLO_TRIANGLE_HPP
#define VK_TUT_HELLO_TRIANGLE_HPP

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstdlib>

class HelloTriangle {
public:
    static int constexpr width  = 800;
    static int constexpr height = 600;

    HelloTriangle() : m_window{initWindow()}, m_instance{createInstace()}
    {}

    auto
    run() -> void
    {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    struct WindowDeleter {
        auto
        operator()(GLFWwindow* window) const noexcept -> void
        {
            glfwDestroyWindow(window);
        }
    };
    using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, WindowDeleter>;
    UniqueGLFWwindow m_window;
    vk::Instance m_instance;

    static auto
    initWindow() noexcept -> UniqueGLFWwindow
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        return UniqueGLFWwindow{
                glfwCreateWindow(width, height, "vk", nullptr, nullptr)};
    }

    static auto
    createInstace() -> vk::Instance
    {
        auto extensionCount = uint32_t{};

        auto const** glfwExtentions =
                glfwGetRequiredInstanceExtensions(&extensionCount);

        auto constexpr appInfo = vk::ApplicationInfo(
                "HelloTriangle",
                1,
                "vkTut",
                1,
                VK_API_VERSION_1_2);

        auto const instanceCreateInfo = vk::InstanceCreateInfo(
                {},
                &appInfo,
                {},
                {},
                extensionCount,
                glfwExtentions);

        auto instance = vk::Instance{};
        auto const result =
                vk::createInstance(&instanceCreateInfo, nullptr, &instance);

        if(result != vk::Result::eSuccess) {
            throw std::runtime_error("Vk instance could not be created");
        }

        return instance;
    }

    auto
    initVulkan() -> void
    {}

    auto
    mainLoop() -> void
    {
        while(glfwWindowShouldClose(m_window.get()) == 0) {
            glfwPollEvents();
        }
    }

    static auto
    cleanup() -> void
    {
        glfwTerminate();
    }
};

#endif    // VK_TUT_HELLO_TRIANGLE_HPP
