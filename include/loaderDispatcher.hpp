#ifndef VK_TUT_LOADER_DISPATCHER_HPP
#define VK_TUT_LOADER_DISPATCHER_HPP

#include <vulkan/vulkan.hpp>

#include <functional>

namespace vulkanUtils {
class DynamicFuncDispatcher {
public:
    explicit DynamicFuncDispatcher(vk::Instance const& instance);

    [[nodiscard]] auto
    boundInstance() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    operator*() const noexcept -> vk::DispatchLoaderDynamic const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::DispatchLoaderDynamic const*;

private:
    vk::DynamicLoader const m_loader;
    vk::DispatchLoaderDynamic const m_dispatcher;

    std::reference_wrapper<vk::Instance const> const m_boundInstance;
};
}    // namespace vulkanUtils

#endif    // VK_TUT_LOADER_DISPATCHER_HPP
