cmake_minimum_required(VERSION 3.24)
project(WindEffects LANGUAGES CXX C)

# C++23 Standard Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Dependency Management: FetchContent
# -----------------------------------------------------------------------------
include(FetchContent)

# 1. Vulkan Headers
message(STATUS "Fetching Vulkan-Headers...")
FetchContent_Declare(
    vulkan_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG v1.3.290
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(vulkan_headers)

# 2. Volk (Vulkan Meta-Loader)
message(STATUS "Fetching Volk...")
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(volk)

# 3. SDL3
message(STATUS "Fetching SDL3...")
# Disable unneeded components of SDL3 to minimize compilation time
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# 5. GLM
message(STATUS "Fetching GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# 6. Dear ImGui (Docking Branch)
message(STATUS "Fetching Dear ImGui (Docking Branch)...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# Define ImGui Library Target
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL3::Headers Vulkan::Headers volk)
# Define IMGUI_IMPL_VULKAN_NO_PROTOTYPES because we use Volk
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)

# 7. stb (for stb_truetype.h)
message(STATUS "Fetching stb...")
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# Download Roboto-Regular.ttf font file
set(FONT_DIR "${CMAKE_BINARY_DIR}/bin/Fonts")
set(FONT_FILE "${FONT_DIR}/Roboto-Regular.ttf")
if(NOT EXISTS "${FONT_FILE}")
    message(STATUS "Downloading Roboto-Regular.ttf...")
    file(DOWNLOAD 
        "https://raw.githubusercontent.com/google/fonts/main/ofl/roboto/Roboto-Regular.ttf"
        "${FONT_FILE}"
        SHOW_PROGRESS
    )
endif()


# -----------------------------------------------------------------------------
# Shader Compiler Setup (glslangValidator)
# -----------------------------------------------------------------------------
set(GLSLANG_DIR "${CMAKE_BINARY_DIR}/glslang")
set(GLSLANG_VALIDATOR "${GLSLANG_DIR}/bin/glslangValidator.exe")

if(NOT EXISTS "${GLSLANG_VALIDATOR}")
    message(STATUS "Downloading glslangValidator.exe...")
    file(DOWNLOAD 
        "https://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-windows-x64-Release.zip" 
        "${CMAKE_BINARY_DIR}/glslang.zip" 
        SHOW_PROGRESS
    )
    file(ARCHIVE_EXTRACT 
        INPUT "${CMAKE_BINARY_DIR}/glslang.zip" 
        DESTINATION "${GLSLANG_DIR}"
    )
    if(EXISTS "${GLSLANG_VALIDATOR}")
        message(STATUS "glslangValidator successfully installed at: ${GLSLANG_VALIDATOR}")
    else()
        message(FATAL_ERROR "Failed to install glslangValidator.exe")
    endif()
endif()

# Helper macro/function to compile shaders to SPV
function(compile_shader SHADER_FILE)
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(SPV_FILE "${CMAKE_BINARY_DIR}/bin/Shaders/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/Shaders"
        COMMAND ${GLSLANG_VALIDATOR} -V -o ${SPV_FILE} ${SHADER_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "Compiling shader ${SHADER_NAME} to SPIR-V..."
    )
    # Expose output shaders to parent scope
    set(SPV_SHADERS_OUT ${SPV_SHADERS_OUT} ${SPV_FILE} PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# Main Application Target
# -----------------------------------------------------------------------------
# Define files (includes Core Logger)
file(GLOB_RECURSE SOURCES 
    "Source/*.cpp"
    "Source/*.hpp"
)

# Shaders
file(GLOB_RECURSE SHADER_SOURCES
    "Source/Shaders/*.vert"
    "Source/Shaders/*.frag"
)

# Compile all shaders
set(SPV_SHADERS_OUT "")
foreach(SHADER IN LISTS SHADER_SOURCES)
    compile_shader(${SHADER})
endforeach()

# Add executable
add_executable(WindEffects ${SOURCES} ${SPV_SHADERS_OUT})

# Include Directories
target_include_directories(WindEffects PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${stb_SOURCE_DIR}
)

# Link Libraries
target_link_libraries(WindEffects PRIVATE 
    imgui
    SDL3::SDL3
    Vulkan::Headers
    volk
    glm::glm
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(WindEffects PRIVATE /W4 /WX /permissive-)
    # Disable some noisy warnings for external libraries if needed
    target_compile_definitions(WindEffects PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(WindEffects PRIVATE dbghelp)
endif()

# Set working directory for debugging
set_target_properties(WindEffects PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
