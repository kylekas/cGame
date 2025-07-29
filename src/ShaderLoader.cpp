#include "ShaderLoader.h"
#include <fstream>
#include <stdexcept>

std::vector<char> ShaderLoader::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    
    return buffer;
}

vk::ShaderModule ShaderLoader::createShaderModule(vk::Device device, const std::vector<char>& code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    return device.createShaderModule(createInfo);
}

vk::ShaderModule ShaderLoader::loadShader(vk::Device device, const std::string& filename) {
    auto code = readFile(filename);
    return createShaderModule(device, code);
} 