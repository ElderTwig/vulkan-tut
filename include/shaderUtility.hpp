#ifndef VK_TUT_SHADER_UTILITY_HPP
#define VK_TUT_SHADER_UTILITY_HPP

#include <string_view>
#include <vulkan/vulkan.hpp>

#include <vector>
#include <string>

namespace shaderUtils {

enum class ShaderType : VkShaderStageFlags {
    Vertex   = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
    Fragment = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
};

[[nodiscard]] auto
create_shader_module(
        vk::UniqueDevice const& logicalDevice,
        ShaderType type,
        std::string const& name) -> vk::UniqueShaderModule;

template<ShaderType _type>
struct [[nodiscard]] Shader
{
    static ShaderType constexpr type = _type;
    static vk::ShaderStageFlagBits constexpr vkType =
            static_cast<vk::ShaderStageFlagBits>(type);

    Shader(vk::UniqueDevice const& logicalDevice, std::string shaderName) :
                name{std::move(shaderName)},
                module{create_shader_module(logicalDevice, type, name)}
    {}

    std::string const name;
    vk::UniqueShaderModule const module;
};

using VertexShader   = Shader<ShaderType::Vertex>;
using FragmentShader = Shader<ShaderType::Fragment>;

[[nodiscard]] auto
shader_stage_creation_info(
        vk::UniqueShaderModule const& shaderModule,
        ShaderType shaderType,
        std::string_view stageName) -> vk::PipelineShaderStageCreateInfo;

}    // namespace shaderUtils

#endif    // VK_TUT_SHADER_UTILITY_HPP
