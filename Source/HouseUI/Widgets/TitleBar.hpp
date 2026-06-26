#pragma once

#include "../Core/Widget.hpp"
#include "MenuBar.hpp"
#include <SDL3/SDL.h>
#include <string>

namespace HouseEngine::UI {

class TitleBar : public Widget {
public:
    TitleBar(SDL_Window* window, const std::string& title, std::shared_ptr<MenuBar> menuBar);
    virtual ~TitleBar() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void OnMouseDown(const MouseEvent& event) override;
    void OnMouseMove(const MouseEvent& event) override;

    SDL_HitTestResult HitTest(SDL_Point point);

private:
    SDL_Window* m_Window = nullptr;
    std::string m_Title;
    std::shared_ptr<MenuBar> m_MenuBar;

    // Window control geometries
    Rect m_MinimizeRect;
    Rect m_MaximizeRect;
    Rect m_CloseRect;

    // Hover states
    int m_HoveredControl = -1; // 0=min, 1=max, 2=close, -1=none
};

} // namespace HouseEngine::UI
