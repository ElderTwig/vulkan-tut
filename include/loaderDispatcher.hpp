#ifndef VK_TUT_LOADER_DISPATCHER_HPP
#define VK_TUT_LOADER_DISPATCHER_HPP

#include <vulkan/vulkan.hpp>

namespace vulkanUtils {
class DynamicFuncDispatcher {
public:
    explicit DynamicFuncDispatcher(vk::Instance const& instance);

    [[nodiscard]] constexpr auto
    operator*() const noexcept -> vk::DispatchLoaderDynamic const&
    {
        return m_dispatcher;
    }

    [[nodiscard]] constexpr auto
    operator->() const noexcept -> vk::DispatchLoaderDynamic const*
    {
        return &m_dispatcher;
    }

private:
    vk::DynamicLoader const m_loader;
    vk::DispatchLoaderDynamic const m_dispatcher;
};
}    // namespace vulkanUtils

#endif    // VK_TUT_LOADER_DISPATCHER_HPP
