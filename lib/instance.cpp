#include "instance.hpp"

namespace vulkanUtils {
auto
create_instance(
        std::vector<char const*> const& validationLayers,
        std::vector<char const*> const& extensions)
{
    auto constexpr applicationInfo = vk::ApplicationInfo{
            "VkTut",
            VK_MAKE_VERSION(1, 0, 0),
            "VkTut",
            VK_MAKE_VERSION(1, 0, 0),
            VK_MAKE_VERSION(1, 0, 0)};

    auto const creationInfo = vk::InstanceCreateInfo{
            {},
            &applicationInfo,
            static_cast<uint32_t>(validationLayers.size()),
            validationLayers.data(),
            static_cast<uint32_t>(extensions.size()),
            extensions.data()};

    return vk::createInstanceUnique(creationInfo);
}

Instance::Instance(
        std::vector<char const*> validationLayers,
        std::vector<char const*> extensions) :
            m_extensions{std::move(extensions)},
            m_validationLayers{std::move(validationLayers)},
            m_instance{create_instance(m_validationLayers, m_extensions)}
{}
}    // namespace vulkanUtils
