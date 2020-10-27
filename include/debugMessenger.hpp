#ifndef VK_TUT_DEBUG_MESSENGER_HPP
#define VK_TUT_DEBUG_MESSENGER_HPP

#include "loaderDispatcher.hpp"

#include <vulkan/vulkan.hpp>

#include <functional>

namespace vulkanUtils {
class DebugMessenger {
public:
    explicit DebugMessenger(DynamicFuncDispatcher const& dispatcher);

    [[nodiscard]] constexpr auto
    boundInstance() const noexcept -> vk::Instance const&
    {
        return m_boundInstance.get();
    }

    [[nodiscard]] constexpr auto
    boundDispatcher() const noexcept -> vk::DispatchLoaderDynamic const&
    {
        return m_boundFuncDispatcher.get();
    }

    [[nodiscard]] constexpr auto
    operator*() const noexcept -> vk::DebugUtilsMessengerEXT const&
    {
        return *m_debugMessenger;
    }

    [[nodiscard]] constexpr auto
    operator->() const noexcept -> vk::DebugUtilsMessengerEXT const*
    {
        return &m_debugMessenger.get();
    }

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
