#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class ShaderLoader {
public:
    static std::vector<char> readFile(const std::string& filename);
    static vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);
    static vk::ShaderModule loadShader(vk::Device device, const std::string& filename);
}; 