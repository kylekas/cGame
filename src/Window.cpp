#include "Window.h"
#include <iostream>

Window::Window() : m_window(nullptr), m_width(0), m_height(0), m_initialized(false) {
}

Window::~Window() {
    cleanup();
}

bool Window::initialize(int width, int height, const std::string& title) {
    m_width = width;
    m_height = height;
    m_title = title;

    // Configure GLFW
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    // Set callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    m_initialized = true;
    return true;
}

void Window::cleanup() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    m_initialized = false;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::getMousePosition(double& x, double& y) const {
    glfwGetCursorPos(m_window, &x, &y);
}

// Static callback functions
void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        // TODO: Handle resize in renderer
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; // Suppress unused parameter warning
    (void)mods;     // Suppress unused parameter warning
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // TODO: Add more key handling
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)window;  // Suppress unused parameter warning
    (void)button;  // Suppress unused parameter warning
    (void)action;  // Suppress unused parameter warning
    (void)mods;    // Suppress unused parameter warning
    // TODO: Add mouse button handling
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window; // Suppress unused parameter warning
    (void)xpos;   // Suppress unused parameter warning
    (void)ypos;   // Suppress unused parameter warning
    // TODO: Add mouse movement handling
} 