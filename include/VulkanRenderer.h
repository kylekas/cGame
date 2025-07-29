#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <memory>
#include "Vertex.h"

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    // Initialize Vulkan
    bool initialize(GLFWwindow* window);
    void cleanup();

    // Main rendering functions
    void beginFrame();
    void endFrame();
    void drawFrame();

    // Getters
    bool isInitialized() const { return m_initialized; }
    vk::Device getDevice() const { return m_device; }
    vk::PhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

private:
    // Vulkan instance and devices
    vk::Instance m_instance;
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_device;
    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;

    // Surface and swap chain
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::ImageView> m_swapchainImageViews;
    vk::Format m_swapchainImageFormat;
    vk::Extent2D m_swapchainExtent;

    // Render pass and framebuffers
    vk::RenderPass m_renderPass;
    std::vector<vk::Framebuffer> m_swapchainFramebuffers;

    // Pipeline
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_graphicsPipeline;
    
    // Shaders
    vk::ShaderModule m_vertexShaderModule;
    vk::ShaderModule m_fragmentShaderModule;
    
    // Vertex buffer
    vk::Buffer m_vertexBuffer;
    vk::DeviceMemory m_vertexBufferMemory;

    // Command pool and buffers
    vk::CommandPool m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;

    // Synchronization
    std::vector<vk::Semaphore> m_imageAvailableSemaphores;
    std::vector<vk::Semaphore> m_renderFinishedSemaphores;
    std::vector<vk::Fence> m_inFlightFences;
    size_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;

    // Window reference
    GLFWwindow* m_window;

    // State
    bool m_initialized = false;

    // Private helper functions
    bool createInstance();
    bool setupDebugMessenger();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createSurface();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createGraphicsPipeline();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();
    bool createVertexBuffer();
    void cleanupVertexBuffer();

    // Utility functions
    bool isDeviceSuitable(vk::PhysicalDevice device);
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    bool checkValidationLayerSupport();
    bool checkSwapChainSupport();
    std::vector<const char*> getRequiredExtensions();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
}; 