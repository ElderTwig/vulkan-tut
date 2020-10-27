#include "debugMessenger.hpp"

#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT /*unused*/,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/)
{
    if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << pCallbackData->pMessage << '\n';
    }

    return (VkBool32) false;
}

using SeverityFlags          = vk::DebugUtilsMessageSeverityFlagBitsEXT;
auto constexpr severityFlags = SeverityFlags::eError | SeverityFlags::eInfo
                               | SeverityFlags::eVerbose
                               | SeverityFlags::eWarning;

using TypeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT;
auto constexpr typeFlags =
        TypeFlags::eGeneral | TypeFlags::ePerformance | TypeFlags::eValidation;

auto constexpr debugMessengerCreationInfo =
        vk::DebugUtilsMessengerCreateInfoEXT{
                {},
                severityFlags,
                typeFlags,
                &debugCallback};

namespace vulkanUtils {

DebugMessenger::DebugMessenger(
        vk::Instance const& instance,
        vk::DispatchLoaderDynamic const& dispatcher) :
            m_debugMessenger{instance.createDebugUtilsMessengerEXTUnique(
                    debugMessengerCreationInfo,
                    nullptr,
                    dispatcher)},
            m_boundFuncDispatcher{dispatcher},
            m_boundInstance{instance}
{}

[[nodiscard]] auto
DebugMessenger::boundInstance() const noexcept -> vk::Instance const&
{
    return m_boundInstance.get();
}

[[nodiscard]] auto
DebugMessenger::boundDispatcher() const noexcept
        -> vk::DispatchLoaderDynamic const&
{
    return m_boundFuncDispatcher.get();
}

[[nodiscard]] auto
DebugMessenger::operator*() const noexcept -> vk::DebugUtilsMessengerEXT const&
{
    return *m_debugMessenger;
}

[[nodiscard]] auto
DebugMessenger::operator->() const noexcept -> vk::DebugUtilsMessengerEXT const*
{
    return &m_debugMessenger.get();
}

}    // namespace vulkanUtils
