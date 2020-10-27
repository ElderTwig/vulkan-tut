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

}    // namespace vulkanUtils
