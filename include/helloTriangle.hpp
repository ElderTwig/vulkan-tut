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
    initWindow() noexcept -> UniqueGLFWwindow;

    static auto
    createInstace() -> vk::Instance;

    auto
    initVulkan() -> void;

    auto
    mainLoop() -> void;

    static auto
    cleanup() -> void;
};

#endif    // VK_TUT_HELLO_TRIANGLE_HPP
