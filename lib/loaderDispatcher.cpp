#include "loaderDispatcher.hpp"

namespace vulkanUtils {

DynamicFuncDispatcher::DynamicFuncDispatcher(vk::Instance const& instance) :
            m_loader{},
            m_dispatcher{
                    instance,
                    m_loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
                            "vkGetInstanceProcAddr")},
            m_boundInstance{instance}
{}

[[nodiscard]] auto
DynamicFuncDispatcher::boundInstance() const noexcept -> vk::Instance const&
{
    return m_boundInstance.get();
}

[[nodiscard]] auto
DynamicFuncDispatcher::operator*() const noexcept
        -> vk::DispatchLoaderDynamic const&
{
    return m_dispatcher;
}

[[nodiscard]] auto
DynamicFuncDispatcher::operator->() const noexcept
        -> vk::DispatchLoaderDynamic const*
{
    return &m_dispatcher;
}

}    // namespace vulkanUtils
