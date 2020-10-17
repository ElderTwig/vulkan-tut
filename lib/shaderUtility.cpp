#include "shaderUtility.hpp"

#include <gsl/gsl>

#include <vector>
#include <fstream>
#include <string>

namespace shaderUtils {
using namespace std::literals;

auto const shaderPath      = "shaders/"s;
auto const shaderBuildPath = shaderPath + "build/"s;

[[nodiscard]] auto
create_shader_module(
        vk::UniqueDevice const& logicalDevice,
        ShaderType const type,
        std::string const& name) -> vk::UniqueShaderModule
{
    auto const typeExtention =
            gsl::span{type == ShaderType::Vertex ? ".vert.spv" : ".frag.spv"};

    auto const filePath = shaderBuildPath + name + typeExtention.data();
    auto shaderFile = std::ifstream(filePath, std::ios::ate | std::ios::binary);

    if(!shaderFile.is_open()) {
        throw std::runtime_error("No binary of given shader exists");
    }

    auto const fileLength = shaderFile.tellg();

    auto byteCode = std::vector<char>(fileLength);

    shaderFile.seekg(0);
    shaderFile.read(byteCode.data(), fileLength);

    auto const creationInfo = vk::ShaderModuleCreateInfo(
            {},
            byteCode.size(),
            reinterpret_cast<uint32_t const*>(byteCode.data()));

    return logicalDevice->createShaderModuleUnique(creationInfo);
}

[[nodiscard]] auto
shader_stage_creation_info(
        vk::UniqueShaderModule const& shaderModule,
        ShaderType shaderType,
        std::string_view stageName) -> vk::PipelineShaderStageCreateInfo
{
    return vk::PipelineShaderStageCreateInfo(
            {},
            static_cast<vk::ShaderStageFlagBits>(shaderType),
            shaderModule.get(),
            stageName.begin());
}
}    // namespace shaderUtils
