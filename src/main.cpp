#include "Window.h"
#include "VulkanRenderer.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Create window
        Window window;
        if (!window.initialize(800, 600, "cGame - Vulkan")) {
            throw std::runtime_error("Failed to create window");
        }

        // Initialize Vulkan renderer
        VulkanRenderer renderer;
        if (!renderer.initialize(window.getWindow())) {
            throw std::runtime_error("Failed to initialize Vulkan renderer");
        }

        std::cout << "Vulkan game initialized successfully!" << std::endl;

        // Main game loop
        while (!window.shouldClose()) {
            // Poll events
            window.pollEvents();

            // Begin frame
            renderer.beginFrame();

            // Draw frame
            renderer.drawFrame();

            // End frame
            renderer.endFrame();
        }

        // Cleanup
        renderer.cleanup();
        window.cleanup();
        glfwTerminate();

        std::cout << "Game closed successfully!" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
} 