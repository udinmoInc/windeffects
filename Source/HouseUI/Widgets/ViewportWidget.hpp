#pragma once

#include <volk.h>
#include "../Core/Widget.hpp"
#include <memory>

namespace HouseEngine {
class Renderer;
class EditorCamera;
class Scene;
}

namespace HouseEngine::UI {

class UIRenderer;

class ViewportWidget : public Widget {
public:
    ViewportWidget(const std::shared_ptr<Renderer>& renderer,
                   const std::shared_ptr<EditorCamera>& camera,
                   const std::shared_ptr<Scene>& scene,
                   UIRenderer* uiRenderer = nullptr);
    virtual ~ViewportWidget();

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void OnMouseDown(const MouseEvent& event) override;
    void OnMouseMove(const MouseEvent& event) override;
    void OnMouseUp(const MouseEvent& event) override;
    void OnMouseWheel(const MouseEvent& event) override;

    void Tick(float deltaTime) override;

private:
    std::shared_ptr<Renderer> m_Renderer;
    std::shared_ptr<EditorCamera> m_Camera;
    std::shared_ptr<Scene> m_Scene;
    UIRenderer* m_uiRenderer = nullptr;

    VkDescriptorSet m_ViewportTextureSet = VK_NULL_HANDLE;

    // Mouse tracking
    Point m_LastMousePos;
    bool m_RightMouseDown = false;
    bool m_LeftMouseDown = false;
    bool m_MiddleMouseDown = false;

    float m_FPS = 0.0f;
    float m_FrameTime = 0.0f;
};

} // namespace HouseEngine::UI
