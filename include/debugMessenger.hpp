#ifndef VK_TUT_DEBUG_MESSENGER_HPP
#define VK_TUT_DEBUG_MESSENGER_HPP

#include <vulkan/vulkan.hpp>

#include <functional>

namespace vulkanUtils {

class DebugMessenger {
public:
    explicit DebugMessenger(
            vk::Instance const& instance,
            vk::DispatchLoaderDynamic const& dispatcher);

    [[nodiscard]] auto
    boundInstance() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    boundDispatcher() const noexcept -> vk::DispatchLoaderDynamic const&;

    [[nodiscard]] auto
    operator*() const noexcept -> vk::DebugUtilsMessengerEXT const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::DebugUtilsMessengerEXT const*;

private:
    vk::UniqueHandle<
            vk::DebugUtilsMessengerEXT,
            vk::DispatchLoaderDynamic> const m_debugMessenger;

    std::reference_wrapper<vk::DispatchLoaderDynamic const> const
            m_boundFuncDispatcher;
    std::reference_wrapper<vk::Instance const> const m_boundInstance;
};

}    // namespace vulkanUtils

#endif    // VK_TUT_DEBUG_MESSENGER_HPP
