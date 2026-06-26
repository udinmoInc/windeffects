#include "Editor.hpp"
#include "../Core/Logger.hpp"

// HouseUI Layout and Widget Headers
#include "HouseUI/Layout/Box.hpp"
#include "HouseUI/Layout/Splitter.hpp"
#include "HouseUI/Layout/ScrollLayout.hpp"
#include "HouseUI/Widgets/Button.hpp"
#include "HouseUI/Widgets/Label.hpp"
#include "HouseUI/Widgets/TextBox.hpp"
#include "HouseUI/Widgets/ViewportWidget.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>

namespace HouseEngine {

Editor::Editor() {
    HE_INFO("Initializing SDL3 platform...");
    InitSDL();
    HE_INFO("SDL3 platform successfully initialized.");

    HE_INFO("Creating Vulkan Context...");
    m_Context = std::make_shared<VulkanContext>(m_Window);
    HE_INFO("Vulkan Context created successfully.");

    HE_INFO("Initializing Vulkan Renderer & Swapchain...");
    m_Renderer = std::make_shared<Renderer>(m_Context, m_Window);
    m_RenderGraph = std::make_shared<RenderGraph>(m_Renderer);
    HE_INFO("Vulkan Renderer & Swapchain ready.");

    HE_INFO("Compiling & loading Scene render pipelines...");
    m_SceneRenderer = std::make_shared<SceneRenderer>(m_Context, m_Renderer->GetOffscreenRenderPass(), m_Renderer->GetCameraDescLayout());
    m_GridRenderer = std::make_shared<GridRenderer>(m_Context, m_Renderer->GetOffscreenRenderPass(), m_Renderer->GetCameraDescLayout());
    HE_INFO("Pipelines loaded successfully.");

    HE_INFO("Initializing Editor Scene Graph...");
    m_Camera = std::make_shared<EditorCamera>();
    m_Scene = std::make_shared<Scene>(m_Context, m_SceneRenderer);
    m_Scene->InitializeDefaultScene(m_Renderer->GetCameraBuffer());
    HE_INFO("Default Scene populated.");

    HE_INFO("Initializing HouseUI Rendering Subsystem...");
    m_UIRenderer = std::make_unique<UI::UIRenderer>();
    if (!m_UIRenderer->Init(m_Context, m_Renderer->GetSwapchainRenderPass())) {
        throw std::runtime_error("Failed to initialize HouseUI Renderer!");
    }
    m_UIEventSystem = std::make_shared<UI::EventSystem>();

    HE_INFO("Building HouseUI Editor Workspace Panel Tree...");
    BuildEditorUI();
    m_UIEventSystem->SetRootWidget(m_RootWidget);

    HE_INFO("WindEffects Engine Editor successfully bootstrapped using HouseUI.");
}

Editor::~Editor() {
    Shutdown();
}

void Editor::InitSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        throw std::runtime_error("Failed to initialize SDL3!");
    }

    m_Window = SDL_CreateWindow(
        "WindEffects Engine - Editor Viewport (HouseUI v0.1)",
        1280, 720,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!m_Window) {
        throw std::runtime_error("Failed to create SDL3 window!");
    }
}

void Editor::BuildEditorUI() {
    // 1. Root Vertical Box
    auto rootVBox = std::make_shared<UI::VerticalBox>();
    rootVBox->SetSpacing(0.0f);

    // 2. Menu Bar (HorizontalBox)
    auto menuBar = std::make_shared<UI::HorizontalBox>();
    menuBar->SetSpacing(8.0f);
    menuBar->SetPadding(UI::Margin{ 6.0f, 2.0f, 6.0f, 2.0f });

    // File Menu exit button
    auto exitBtn = std::make_shared<UI::Button>(" Exit ", [this]() { m_Running = false; });
    // Edit Menu reset camera button
    auto resetCamBtn = std::make_shared<UI::Button>(" Reset Camera ", [this]() { m_Camera->Reset(); });
    // Debug Menu crash test buttons
    auto crashBtn = std::make_shared<UI::Button>(" Trigger Crash (Null Pointer) ", []() {
        volatile int* ptr = nullptr;
        *ptr = 0xDEADBEEF;
    });
    auto throwBtn = std::make_shared<UI::Button>(" Trigger C++ Exception ", []() {
        throw std::runtime_error("Simulated engine exception from HouseUI debug menu.");
    });

    menuBar->AddChild(exitBtn);
    menuBar->AddChild(resetCamBtn);
    menuBar->AddChild(crashBtn);
    menuBar->AddChild(throwBtn);
    rootVBox->AddChild(menuBar);

    // 3. Toolbar (HorizontalBox)
    auto toolbar = std::make_shared<UI::HorizontalBox>();
    toolbar->SetSpacing(6.0f);
    toolbar->SetPadding(UI::Margin{ 6.0f, 4.0f, 6.0f, 4.0f });

    auto addPlaneBtn = std::make_shared<UI::Button>(" Add Plane ", [this]() {
        m_Scene->CreateEntity("Ground Plane", EntityType::Plane, m_Renderer->GetCameraBuffer());
    });
    auto addCubeBtn = std::make_shared<UI::Button>(" Add Cube ", [this]() {
        m_Scene->CreateEntity("Cube", EntityType::Cube, m_Renderer->GetCameraBuffer());
    });
    auto litBtn = std::make_shared<UI::Button>(" Lit ", [this]() {
        for (auto& entity : m_Scene->GetEntities()) entity.Mode = 0;
    });
    auto unlitBtn = std::make_shared<UI::Button>(" Unlit ", [this]() {
        for (auto& entity : m_Scene->GetEntities()) entity.Mode = 1;
    });
    auto wireframeBtn = std::make_shared<UI::Button>(" Wireframe ", [this]() {
        for (auto& entity : m_Scene->GetEntities()) entity.Mode = 2;
    });

    toolbar->AddChild(addPlaneBtn);
    toolbar->AddChild(addCubeBtn);
    toolbar->AddChild(litBtn);
    toolbar->AddChild(unlitBtn);
    toolbar->AddChild(wireframeBtn);
    rootVBox->AddChild(toolbar);

    // 4. Main Splitter Panel (Left sidebar vs Right workspace)
    auto mainSplitter = std::make_shared<UI::Splitter>(UI::Orientation::Horizontal, 0.18f);
    
    // Left Sidebar: Outliner
    auto outlinerPanel = std::make_shared<UI::VerticalBox>();
    outlinerPanel->SetPadding(UI::Margin{ 4.0f, 4.0f, 4.0f, 4.0f });
    outlinerPanel->SetSpacing(4.0f);
    
    auto outlinerHeader = std::make_shared<UI::Label>("Scene Outliner", UI::Color{ 0.95f, 0.8f, 0.2f, 1.0f }, 13.0f);
    outlinerPanel->AddChild(outlinerHeader);

    auto outlinerScroll = std::make_shared<UI::ScrollLayout>();
    m_OutlinerList = std::make_shared<UI::VerticalBox>();
    m_OutlinerList->SetSpacing(2.0f);
    outlinerScroll->SetContent(m_OutlinerList);
    outlinerPanel->AddChild(outlinerScroll);

    mainSplitter->SetFirstChild(outlinerPanel);

    // Right Workspace (Center splits vs Right details)
    auto workspaceRightSplitter = std::make_shared<UI::Splitter>(UI::Orientation::Horizontal, 0.78f);

    // Center layout: Viewport (Top) vs Console (Bottom)
    auto centerSplitter = std::make_shared<UI::Splitter>(UI::Orientation::Vertical, 0.70f);

    auto viewportWidget = std::make_shared<UI::ViewportWidget>(m_Renderer, m_Camera, m_Scene);
    centerSplitter->SetFirstChild(viewportWidget);

    // Console Log panel
    auto consolePanel = std::make_shared<UI::VerticalBox>();
    consolePanel->SetPadding(UI::Margin{ 4.0f, 4.0f, 4.0f, 4.0f });
    consolePanel->SetSpacing(4.0f);
    
    auto consoleHeader = std::make_shared<UI::Label>("Output Log", UI::Color{ 0.95f, 0.8f, 0.2f, 1.0f }, 13.0f);
    consolePanel->AddChild(consoleHeader);

    auto consoleScroll = std::make_shared<UI::ScrollLayout>();
    m_ConsoleList = std::make_shared<UI::VerticalBox>();
    m_ConsoleList->SetSpacing(1.0f);
    consoleScroll->SetContent(m_ConsoleList);
    consolePanel->AddChild(consoleScroll);

    centerSplitter->SetSecondChild(consolePanel);
    workspaceRightSplitter->SetFirstChild(centerSplitter);

    // Right Sidebar: Details/Inspector Panel
    m_InspectorPanel = std::make_shared<UI::VerticalBox>();
    m_InspectorPanel->SetPadding(UI::Margin{ 6.0f, 6.0f, 6.0f, 6.0f });
    m_InspectorPanel->SetSpacing(6.0f);

    workspaceRightSplitter->SetSecondChild(m_InspectorPanel);
    mainSplitter->SetSecondChild(workspaceRightSplitter);

    rootVBox->AddChild(mainSplitter);
    m_RootWidget = rootVBox;

    // Trigger initial panel populate
    UpdateOutlinerPanel();
    UpdateInspectorPanel();
}

void Editor::UpdateOutlinerPanel() {
    m_OutlinerList->ClearChildren();
    
    auto& entities = m_Scene->GetEntities();
    int selectedIdx = m_Scene->GetSelectedEntityIndex();

    for (size_t i = 0; i < entities.size(); ++i) {
        std::string prefix = (static_cast<int>(i) == selectedIdx) ? " > " : "   ";
        std::string entityTypeStr = "[Cube] ";
        if (entities[i].Type == EntityType::Plane) entityTypeStr = "[Plane] ";
        else if (entities[i].Type == EntityType::DirectionalLight) entityTypeStr = "[Light] ";
        else if (entities[i].Type == EntityType::CameraIcon) entityTypeStr = "[Camera] ";

        std::string btnText = prefix + entityTypeStr + entities[i].Name;
        auto btn = std::make_shared<UI::Button>(btnText, [this, i]() {
            m_Scene->SetSelectedEntityIndex(static_cast<int>(i));
            UpdateInspectorPanel();
        });
        m_OutlinerList->AddChild(btn);
    }
    m_LastEntityCount = entities.size();
    m_LastSelectedEntity = selectedIdx;
}

void Editor::UpdateInspectorPanel() {
    m_InspectorPanel->ClearChildren();

    auto title = std::make_shared<UI::Label>("Details Inspector", UI::Color{ 0.95f, 0.8f, 0.2f, 1.0f }, 13.0f);
    m_InspectorPanel->AddChild(title);

    int selectedIdx = m_Scene->GetSelectedEntityIndex();
    if (selectedIdx < 0 || selectedIdx >= static_cast<int>(m_Scene->GetEntities().size())) {
        auto noSelectLbl = std::make_shared<UI::Label>("No entity selected.", UI::Color{ 0.6f, 0.6f, 0.6f, 1.0f }, 12.0f);
        m_InspectorPanel->AddChild(noSelectLbl);
        return;
    }

    auto& entity = m_Scene->GetEntities()[selectedIdx];

    auto nameLbl = std::make_shared<UI::Label>("Name: " + entity.Name, UI::Color{ 0.9f, 0.9f, 0.9f, 1.0f }, 13.0f);
    m_InspectorPanel->AddChild(nameLbl);

    // Position fields
    auto posTitle = std::make_shared<UI::Label>("Position (X, Y, Z):", UI::Color{ 0.7f, 0.7f, 0.7f, 1.0f }, 12.0f);
    m_InspectorPanel->AddChild(posTitle);

    auto posBox = std::make_shared<UI::HorizontalBox>();
    posBox->SetSpacing(4.0f);

    auto xBox = std::make_shared<UI::TextBox>(std::to_string(entity.Position.x).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Position.x = std::stof(text); } catch (...) {}
    });
    auto yBox = std::make_shared<UI::TextBox>(std::to_string(entity.Position.y).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Position.y = std::stof(text); } catch (...) {}
    });
    auto zBox = std::make_shared<UI::TextBox>(std::to_string(entity.Position.z).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Position.z = std::stof(text); } catch (...) {}
    });

    posBox->AddChild(xBox);
    posBox->AddChild(yBox);
    posBox->AddChild(zBox);
    m_InspectorPanel->AddChild(posBox);

    // Scale fields
    auto scaleTitle = std::make_shared<UI::Label>("Scale (X, Y, Z):", UI::Color{ 0.7f, 0.7f, 0.7f, 1.0f }, 12.0f);
    m_InspectorPanel->AddChild(scaleTitle);

    auto scaleBox = std::make_shared<UI::HorizontalBox>();
    scaleBox->SetSpacing(4.0f);

    auto sxBox = std::make_shared<UI::TextBox>(std::to_string(entity.Scale.x).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Scale.x = std::stof(text); } catch (...) {}
    });
    auto syBox = std::make_shared<UI::TextBox>(std::to_string(entity.Scale.y).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Scale.y = std::stof(text); } catch (...) {}
    });
    auto szBox = std::make_shared<UI::TextBox>(std::to_string(entity.Scale.z).substr(0, 5), [&entity](const std::string& text) {
        try { entity.Scale.z = std::stof(text); } catch (...) {}
    });

    scaleBox->AddChild(sxBox);
    scaleBox->AddChild(syBox);
    scaleBox->AddChild(szBox);
    m_InspectorPanel->AddChild(scaleBox);

    // Shading options
    auto ShadingLbl = std::make_shared<UI::Label>("Shading Mode:", UI::Color{ 0.7f, 0.7f, 0.7f, 1.0f }, 12.0f);
    m_InspectorPanel->AddChild(ShadingLbl);

    auto shadingBox = std::make_shared<UI::HorizontalBox>();
    shadingBox->SetSpacing(4.0f);

    auto litBtn = std::make_shared<UI::Button>(" Lit ", [&entity]() { entity.Mode = 0; });
    auto unlitBtn = std::make_shared<UI::Button>(" Unlit ", [&entity]() { entity.Mode = 1; });
    auto wireBtn = std::make_shared<UI::Button>(" Wire ", [&entity]() { entity.Mode = 2; });

    shadingBox->AddChild(litBtn);
    shadingBox->AddChild(unlitBtn);
    shadingBox->AddChild(wireBtn);
    m_InspectorPanel->AddChild(shadingBox);

    // Delete button
    auto deleteBtn = std::make_shared<UI::Button>(" Delete Actor ", [this, selectedIdx]() {
        m_Scene->DestroyEntity(static_cast<size_t>(selectedIdx));
        m_Scene->SetSelectedEntityIndex(-1);
        UpdateOutlinerPanel();
        UpdateInspectorPanel();
    });
    m_InspectorPanel->AddChild(deleteBtn);
}

void Editor::UpdateConsolePanel() {
    auto logs = Logger::GetNewLogs();
    if (logs.empty()) return;

    for (const auto& log : logs) {
        UI::Color col = UI::Color{ 0.85f, 0.85f, 0.88f, 1.0f }; // Info
        if (log.level == Logger::Level::Warning) {
            col = UI::Color{ 0.95f, 0.8f, 0.25f, 1.0f }; // Warning yellow
        } else if (log.level == Logger::Level::Error) {
            col = UI::Color{ 0.9f, 0.15f, 0.15f, 1.0f }; // Error red
        } else if (log.level == Logger::Level::Debug) {
            col = UI::Color{ 0.3f, 0.6f, 0.9f, 1.0f }; // Debug blue
        }

        auto lbl = std::make_shared<UI::Label>(log.formattedText, col, 12.0f);
        m_ConsoleList->AddChild(lbl);
    }

    // Limit buffer length
    auto& children = m_ConsoleList->GetChildren();
    if (children.size() > 100) {
        m_ConsoleList->ClearChildren();
    }
}

void Editor::Run() {
    MainLoop();
}

void Editor::MainLoop() {
    uint64_t lastTime = SDL_GetPerformanceCounter();
    double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (m_Running) {
        // 1. Process Platform Window Events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                m_Running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_Window)) {
                m_Running = false;
            }

            // Route events to HouseUI EventSystem
            UI::MouseEvent mouseEvent{};
            mouseEvent.altDown = (SDL_GetModState() & SDL_KMOD_ALT) != 0;
            mouseEvent.shiftDown = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
            mouseEvent.ctrlDown = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;

            bool isMouse = false;

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouseEvent.type = UI::MouseEventType::MouseMove;
                mouseEvent.position = UI::Point{ event.motion.x, event.motion.y };
                isMouse = true;
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                mouseEvent.type = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? UI::MouseEventType::MouseDown : UI::MouseEventType::MouseUp;
                mouseEvent.position = UI::Point{ event.button.x, event.button.y };
                if (event.button.button == SDL_BUTTON_LEFT) mouseEvent.button = UI::MouseButton::Left;
                else if (event.button.button == SDL_BUTTON_RIGHT) mouseEvent.button = UI::MouseButton::Right;
                else if (event.button.button == SDL_BUTTON_MIDDLE) mouseEvent.button = UI::MouseButton::Middle;
                isMouse = true;
            } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                // Get mouse coordinates for wheel focus routing
                float mx, my;
                SDL_GetMouseState(&mx, &my);
                mouseEvent.type = UI::MouseEventType::MouseWheel;
                mouseEvent.position = UI::Point{ mx, my };
                mouseEvent.wheelDeltaX = event.wheel.x;
                mouseEvent.wheelDeltaY = event.wheel.y;
                isMouse = true;
            }

            if (isMouse) {
                m_UIEventSystem->ProcessMouseEvent(mouseEvent);
            }

            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                UI::KeyEvent keyEvent{};
                keyEvent.type = (event.type == SDL_EVENT_KEY_DOWN) ? UI::KeyEventType::KeyDown : UI::KeyEventType::KeyUp;
                keyEvent.keycode = event.key.key;
                keyEvent.altDown = (SDL_GetModState() & SDL_KMOD_ALT) != 0;
                keyEvent.shiftDown = (SDL_GetModState() & SDL_KMOD_SHIFT) != 0;
                keyEvent.ctrlDown = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
                m_UIEventSystem->ProcessKeyEvent(keyEvent);
            }
        }

        if (!m_Running) {
            break;
        }

        // Check minimization
        SDL_WindowFlags windowFlags = SDL_GetWindowFlags(m_Window);
        if (windowFlags & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(16);
            continue;
        }

        // 2. Frame timers
        uint64_t now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((now - lastTime) / frequency);
        lastTime = now;

        // Clamp delta time to avoid huge spikes
        if (dt > 0.1f) dt = 0.1f;

        // 3. Update active panels & widgets
        if (m_Scene->GetEntities().size() != m_LastEntityCount || m_Scene->GetSelectedEntityIndex() != m_LastSelectedEntity) {
            UpdateOutlinerPanel();
            UpdateInspectorPanel();
        }
        UpdateConsolePanel();

        // 4. Update cameras & ticks
        m_Camera->Update(dt);
        m_Scene->Update();

        // Tick retained widgets
        m_RootWidget->Tick(dt);

        // 5. Update Uniform buffers
        Renderer::CameraUniform cameraUBO{};
        cameraUBO.view = m_Camera->GetViewMatrix();
        cameraUBO.proj = m_Camera->GetProjectionMatrix();
        cameraUBO.pos = m_Camera->GetPosition();
        cameraUBO.padding = 0.0f;
        m_Renderer->UpdateCameraBuffer(cameraUBO);

        // 6. Draw Frame
        if (m_Renderer->BeginFrame()) {
            VkCommandBuffer cmd = m_Renderer->GetCommandBuffer();

            // Pass 1: Offscreen Viewport Scene Pass
            m_RenderGraph->BeginOffscreenPass(cmd);
            m_SceneRenderer->DrawSkybox(cmd, m_Renderer->GetCameraDescSet());
            m_GridRenderer->Draw(cmd, m_Renderer->GetCameraDescSet());

            for (const auto& entity : m_Scene->GetEntities()) {
                std::string meshName = "Cube";
                if (entity.Type == EntityType::Plane) {
                    meshName = "Plane";
                }
                m_SceneRenderer->DrawMesh(cmd, meshName, entity.DescriptorSet, entity.Mode);
            }
            m_RenderGraph->EndOffscreenPass(cmd);

            // Pass 2: UI Swapchain Pass
            m_RenderGraph->BeginSwapchainPass(cmd);
            
            // Draw HouseUI widget tree
            m_UIRenderer->Render(
                cmd,
                m_Renderer->GetSwapchainWidth(),
                m_Renderer->GetSwapchainHeight(),
                m_RootWidget
            );

            m_RenderGraph->EndSwapchainPass(cmd);

            // Present Swapchain Image
            m_Renderer->EndFrame();
        }
    }
}

void Editor::Shutdown() {
    vkDeviceWaitIdle(m_Context->GetDevice());

    // Destroy HouseUI
    m_OutlinerList.reset();
    m_InspectorPanel.reset();
    m_ConsoleList.reset();
    m_ContentList.reset();
    m_RootWidget.reset();
    
    if (m_UIRenderer) {
        m_UIRenderer->Shutdown();
        m_UIRenderer.reset();
    }
    m_UIEventSystem.reset();

    // Destroy Scene renderers
    m_Scene.reset();
    m_Camera.reset();
    m_GridRenderer.reset();
    m_SceneRenderer.reset();
    m_RenderGraph.reset();
    m_Renderer.reset();
    m_Context.reset();

    // Destroy SDL3
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
    SDL_Quit();
}

} // namespace HouseEngine
