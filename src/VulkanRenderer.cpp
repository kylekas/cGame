#include "VulkanRenderer.h"
#include "ShaderLoader.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>

// Validation layers
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VulkanRenderer::VulkanRenderer() : m_window(nullptr), m_initialized(false) {
}

VulkanRenderer::~VulkanRenderer() {
    cleanup();
}

bool VulkanRenderer::initialize(GLFWwindow* window) {
    m_window = window;
    
    try {
        std::cout << "Creating Vulkan instance..." << std::endl;
        if (!createInstance()) return false;
        
        std::cout << "Picking physical device..." << std::endl;
        if (!pickPhysicalDevice()) return false;
        
        std::cout << "Creating logical device..." << std::endl;
        if (!createLogicalDevice()) return false;
        
        std::cout << "Creating surface..." << std::endl;
        if (!createSurface()) return false;
        
        std::cout << "Checking swap chain support..." << std::endl;
        if (!checkSwapChainSupport()) return false;
        
        std::cout << "Creating swap chain..." << std::endl;
        if (!createSwapChain()) return false;
        
        std::cout << "Creating image views..." << std::endl;
        if (!createImageViews()) return false;
        
        std::cout << "Creating render pass..." << std::endl;
        if (!createRenderPass()) return false;
        
        std::cout << "Creating vertex buffer..." << std::endl;
        if (!createVertexBuffer()) return false;
        
        std::cout << "Creating graphics pipeline..." << std::endl;
        if (!createGraphicsPipeline()) return false;
        
        std::cout << "Creating framebuffers..." << std::endl;
        if (!createFramebuffers()) return false;
        
        std::cout << "Creating command pool..." << std::endl;
        if (!createCommandPool()) return false;
        
        std::cout << "Creating command buffers..." << std::endl;
        if (!createCommandBuffers()) return false;
        
        std::cout << "Creating sync objects..." << std::endl;
        if (!createSyncObjects()) return false;
        
        std::cout << "Vulkan initialization completed successfully!" << std::endl;
        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Vulkan initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void VulkanRenderer::cleanup() {
    if (!m_initialized) return;
    
    m_device.waitIdle();
    
    // Cleanup synchronization objects
    for (size_t i = 0; i < m_inFlightFences.size(); i++) {
        m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
        m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
        m_device.destroyFence(m_inFlightFences[i]);
    }
    
    // Cleanup command pool
    m_device.destroyCommandPool(m_commandPool);
    
    // Cleanup vertex buffer
    cleanupVertexBuffer();
    
    // Cleanup shaders
    m_device.destroyShaderModule(m_vertexShaderModule);
    m_device.destroyShaderModule(m_fragmentShaderModule);
    
    // Cleanup pipeline
    m_device.destroyPipeline(m_graphicsPipeline);
    m_device.destroyPipelineLayout(m_pipelineLayout);
    
    // Cleanup render pass
    m_device.destroyRenderPass(m_renderPass);
    
    // Cleanup framebuffers
    for (auto framebuffer : m_swapchainFramebuffers) {
        m_device.destroyFramebuffer(framebuffer);
    }
    
    // Cleanup image views
    for (auto imageView : m_swapchainImageViews) {
        m_device.destroyImageView(imageView);
    }
    
    // Cleanup swap chain
    m_device.destroySwapchainKHR(m_swapchain);
    
    // Cleanup surface
    m_instance.destroySurfaceKHR(m_surface);
    
    // Cleanup device
    m_device.destroy();
    
    // Cleanup instance
    m_instance.destroy();
    
    m_initialized = false;
}

void VulkanRenderer::beginFrame() {
    // Wait for the previous frame to finish
    vk::Result result = m_device.waitForFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fences");
    }
    
    // Acquire the next image from the swap chain
    result = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, 
        m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex);
    
    if (result == vk::Result::eErrorOutOfDateKHR) {
        // TODO: Handle swap chain recreation
        return;
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }
    
    // Reset the fence
    result = m_device.resetFences(1, &m_inFlightFences[m_currentFrame]);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to reset fences");
    }
}

void VulkanRenderer::endFrame() {
    // Submit the command buffer
    vk::SubmitInfo submitInfo{};
    vk::Result result;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame];
    
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentImageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
    
    result = m_graphicsQueue.submit(1, &submitInfo, m_inFlightFences[m_currentFrame]);
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to submit command buffer");
    }
    
    // Present the image
    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImageIndex;
    
    result = m_presentQueue.presentKHR(&presentInfo);
    
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        // TODO: Handle swap chain recreation
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present swap chain image");
    }
    
    m_currentFrame = (m_currentFrame + 1) % m_inFlightFences.size();
}

void VulkanRenderer::drawFrame() {
    // Safety check
    if (!m_initialized || m_currentImageIndex >= m_commandBuffers.size()) {
        return;
    }
    
    // Record command buffer
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    
    m_commandBuffers[m_currentImageIndex].begin(beginInfo);
    
    // Begin render pass
    vk::RenderPassBeginInfo renderPassInfo{};
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[m_currentImageIndex];
    renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderPassInfo.renderArea.extent = m_swapchainExtent;
    
    vk::ClearValue clearColor;
    clearColor.color = vk::ClearColorValue{ 0.0f, 0.5f, 1.0f, 1.0f }; // Bright blue for Hello World
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    m_commandBuffers[m_currentImageIndex].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    
    // For now, just clear the screen with a "Hello World" color (bright blue)
    // We'll add actual rendering later
    std::cout << "Rendering Hello World (blue background)..." << std::endl;
    
    // End render pass
    m_commandBuffers[m_currentImageIndex].endRenderPass();
    
    // End command buffer
    m_commandBuffers[m_currentImageIndex].end();
}

bool VulkanRenderer::createInstance() {
    // Check validation layer support
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available");
    }
    
    // Application info
    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "cGame";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    // Instance create info
    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    
    // Extensions
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    // Validation layers
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create instance
    m_instance = vk::createInstance(createInfo);
    return true;
}

bool VulkanRenderer::pickPhysicalDevice() {
    std::cout << "Enumerating physical devices..." << std::endl;
    auto devices = m_instance.enumeratePhysicalDevices();
    std::cout << "Found " << devices.size() << " physical devices" << std::endl;
    
    for (const auto& device : devices) {
        std::cout << "Checking device..." << std::endl;
        if (isDeviceSuitable(device)) {
            std::cout << "Selected device" << std::endl;
            m_physicalDevice = device;
            return true;
        }
    }
    
    throw std::runtime_error("Failed to find a suitable GPU");
}

bool VulkanRenderer::createLogicalDevice() {
    std::cout << "Getting queue family properties..." << std::endl;
    // Queue families
    auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();
    std::cout << "Found " << queueFamilies.size() << " queue families" << std::endl;
    
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        std::cout << "Checking queue family " << i << std::endl;
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
            std::cout << "Found graphics queue family: " << i << std::endl;
        }
        
        // We'll check surface support after creating the surface
        // For now, just assume the graphics queue can also present
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            presentFamily = i;
            std::cout << "Using graphics queue for present: " << i << std::endl;
        }
        
        if (graphicsFamily.has_value() && presentFamily.has_value()) {
            break;
        }
    }
    
    if (!graphicsFamily.has_value() || !presentFamily.has_value()) {
        throw std::runtime_error("Failed to find suitable queue families");
    }
    
    // Create queues
    std::cout << "Creating queue create infos..." << std::endl;
    std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Device features
    std::cout << "Setting up device features..." << std::endl;
    vk::PhysicalDeviceFeatures deviceFeatures{};
    
    // Device create info
    std::cout << "Creating device create info..." << std::endl;
    vk::DeviceCreateInfo createInfo{};
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    // Create device
    std::cout << "Creating logical device..." << std::endl;
    m_device = m_physicalDevice.createDevice(createInfo);
    std::cout << "Getting queues..." << std::endl;
    m_graphicsQueue = m_device.getQueue(graphicsFamily.value(), 0);
    m_presentQueue = m_device.getQueue(presentFamily.value(), 0);
    
    std::cout << "Logical device created successfully" << std::endl;
    return true;
}

bool VulkanRenderer::createSurface() {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    m_surface = surface;
    return true;
}

bool VulkanRenderer::createSwapChain() {
    // Get surface capabilities
    auto capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
    auto formats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
    auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
    
    // Choose surface format
    m_swapchainImageFormat = chooseSwapSurfaceFormat(formats).format;
    m_swapchainExtent = chooseSwapExtent(capabilities);
    
    // Choose present mode
    auto presentMode = chooseSwapPresentMode(presentModes);
    
    // Determine number of images
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    // Create swap chain
    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_swapchainImageFormat;
    createInfo.imageColorSpace = chooseSwapSurfaceFormat(formats).colorSpace;
    createInfo.imageExtent = m_swapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    
    // Queue family indices
    auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
        }
        if (m_physicalDevice.getSurfaceSupportKHR(i, m_surface)) {
            presentFamily = i;
        }
    }
    
    if (graphicsFamily.value() != presentFamily.value()) {
        uint32_t queueFamilyIndices[] = { graphicsFamily.value(), presentFamily.value() };
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }
    
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    m_swapchain = m_device.createSwapchainKHR(createInfo);
    m_swapchainImages = m_device.getSwapchainImagesKHR(m_swapchain);
    
    return true;
}

bool VulkanRenderer::createImageViews() {
    m_swapchainImageViews.resize(m_swapchainImages.size());
    
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = m_swapchainImageFormat;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        m_swapchainImageViews[i] = m_device.createImageView(createInfo);
    }
    
    return true;
}

bool VulkanRenderer::createRenderPass() {
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    
    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
    
    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    
    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    m_renderPass = m_device.createRenderPass(renderPassInfo);
    return true;
}

bool VulkanRenderer::createGraphicsPipeline() {
    // For now, create a simple pipeline without shaders to test rendering
    // We'll just clear the screen with a color
    
    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);
    
    std::cout << "Graphics pipeline creation (simplified) completed" << std::endl;
    return true;
}

bool VulkanRenderer::createFramebuffers() {
    m_swapchainFramebuffers.resize(m_swapchainImageViews.size());
    
    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        vk::ImageView attachments[] = { m_swapchainImageViews[i] };
        
        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;
        
        m_swapchainFramebuffers[i] = m_device.createFramebuffer(framebufferInfo);
    }
    
    return true;
}

bool VulkanRenderer::createCommandPool() {
    auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();
    std::optional<uint32_t> graphicsFamily;
    
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
            break;
        }
    }
    
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = graphicsFamily.value();
    
    m_commandPool = m_device.createCommandPool(poolInfo);
    return true;
}

bool VulkanRenderer::createCommandBuffers() {
    m_commandBuffers.resize(m_swapchainFramebuffers.size());
    
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    
    m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
    return true;
}

bool VulkanRenderer::createSyncObjects() {
    m_imageAvailableSemaphores.resize(2);
    m_renderFinishedSemaphores.resize(2);
    m_inFlightFences.resize(2);
    
    vk::SemaphoreCreateInfo semaphoreInfo{};
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    
    for (size_t i = 0; i < 2; i++) {
        m_imageAvailableSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
        m_renderFinishedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
        m_inFlightFences[i] = m_device.createFence(fenceInfo);
    }
    
    return true;
}

// Utility functions
bool VulkanRenderer::isDeviceSuitable(vk::PhysicalDevice device) {
    auto properties = device.getProperties();
    auto features = device.getFeatures();
    
    // Accept any GPU type (including software renderers)
    // Prefer discrete GPU, then integrated, then CPU
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        std::cout << "Found discrete GPU: " << properties.deviceName << std::endl;
    } else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        std::cout << "Found integrated GPU: " << properties.deviceName << std::endl;
    } else if (properties.deviceType == vk::PhysicalDeviceType::eCpu) {
        std::cout << "Found CPU renderer: " << properties.deviceName << std::endl;
    } else {
        std::cout << "Found other device: " << properties.deviceName << std::endl;
    }
    
    // Check for required extensions
    std::cout << "Checking device extensions..." << std::endl;
    if (!checkDeviceExtensionSupport(device)) {
        std::cout << "Device doesn't support required extensions" << std::endl;
        return false;
    }
    std::cout << "Device supports required extensions" << std::endl;
    
    // Note: We'll check swap chain support after creating the surface
    // This is done in a separate function
    
    return true;
}

bool VulkanRenderer::checkDeviceExtensionSupport(vk::PhysicalDevice device) {
    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

bool VulkanRenderer::checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

bool VulkanRenderer::checkSwapChainSupport() {
    auto formats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);
    auto presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
    
    if (formats.empty() || presentModes.empty()) {
        std::cout << "Device doesn't support swap chain" << std::endl;
        return false;
    }
    std::cout << "Device supports swap chain" << std::endl;
    
    return true;
}

vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && 
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }
    
    return availableFormats[0];
}

vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanRenderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        
        vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        
        return actualExtent;
    }
}

bool VulkanRenderer::createVertexBuffer() {
    // Create triangle vertices
    std::vector<Vertex> vertices = {
        {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // Red
        {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},  // Green
        {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}   // Blue
    };
    
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
    // Create buffer
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = bufferSize;
    bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;
    
    m_vertexBuffer = m_device.createBuffer(bufferInfo);
    
    // Allocate memory
    vk::MemoryRequirements memRequirements = m_device.getBufferMemoryRequirements(m_vertexBuffer);
    
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    
    m_vertexBufferMemory = m_device.allocateMemory(allocInfo);
    
    // Bind memory
    m_device.bindBufferMemory(m_vertexBuffer, m_vertexBufferMemory, 0);
    
    // Copy data
    void* data = m_device.mapMemory(m_vertexBufferMemory, 0, bufferSize);
    memcpy(data, vertices.data(), bufferSize);
    m_device.unmapMemory(m_vertexBufferMemory);
    
    std::cout << "Vertex buffer created successfully" << std::endl;
    return true;
}

void VulkanRenderer::cleanupVertexBuffer() {
    if (m_vertexBuffer) {
        m_device.destroyBuffer(m_vertexBuffer);
        m_vertexBuffer = VK_NULL_HANDLE;
    }
    if (m_vertexBufferMemory) {
        m_device.freeMemory(m_vertexBufferMemory);
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type");
} 