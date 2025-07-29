#pragma once

#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window();
    ~Window();

    // Window management
    bool initialize(int width, int height, const std::string& title);
    void cleanup();
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    // Getters
    GLFWwindow* getWindow() const { return m_window; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    bool isInitialized() const { return m_initialized; }

    // Input handling
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getMousePosition(double& x, double& y) const;

private:
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    std::string m_title;
    bool m_initialized;

    // Callback functions
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
}; 