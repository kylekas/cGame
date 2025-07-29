# Collect all source files
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Vulkan::Vulkan
    glfw
    glm::glm
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_link_libraries(${PROJECT_NAME} dl)
elseif(APPLE)
    # macOS-specific settings
endif() 