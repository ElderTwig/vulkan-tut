#ifndef VK_TUT_INSTANCE_HPP
#define VK_TUT_INSTANCE_HPP

#include <vulkan/vulkan.hpp>

#include <vector>
#include <string_view>

namespace vulkanUtils {
class Instance {
public:
    explicit Instance(
            std::vector<char const*> validationLayers,
            std::vector<char const*> extensions);

    [[nodiscard]] constexpr auto
    operator*() const noexcept -> vk::Instance const&
    {
        return *m_instance;
    }

    [[nodiscard]] constexpr auto
    operator->() const noexcept -> vk::Instance const*
    {
        return &m_instance.get();
    }

    [[nodiscard]] constexpr auto
    extensions() const noexcept -> std::vector<char const*> const&
    {
        return m_extensions;
    }

    [[nodiscard]] constexpr auto
    validationLayers() const noexcept -> std::vector<char const*> const&
    {
        return m_validationLayers;
    }

private:
    std::vector<char const*> const m_extensions;
    std::vector<char const*> const m_validationLayers;
    vk::UniqueInstance const m_instance;
};
}    // namespace vulkanUtils

#endif    // VK_TUT_INSTANCE_HPP
