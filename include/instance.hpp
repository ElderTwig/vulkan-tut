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

    [[nodiscard]] auto
    operator*() const noexcept -> vk::Instance const&;

    [[nodiscard]] auto
    operator->() const noexcept -> vk::Instance const*;

    [[nodiscard]] auto
    extensions() const noexcept -> std::vector<char const*> const&;

    [[nodiscard]] auto
    validationLayers() const noexcept -> std::vector<char const*> const&;

private:
    std::vector<char const*> const m_extensions;
    std::vector<char const*> const m_validationLayers;
    vk::UniqueInstance const m_instance;
};
}    // namespace vulkanUtils

#endif    // VK_TUT_INSTANCE_HPP
