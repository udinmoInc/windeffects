#include "TitleBar.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <iostream>

namespace HouseEngine::UI {

TitleBar::TitleBar(SDL_Window* window, const std::string& title, std::shared_ptr<MenuBar> menuBar)
    : m_Window(window), m_Title(title), m_MenuBar(menuBar)
{
    if (m_MenuBar) {
        AddChild(m_MenuBar);
    }
}

Size TitleBar::Measure(const Size& availableSize) {
    float height = 32.0f; // Fixed height for title bar
    
    if (m_MenuBar) {
        // Menu bar gets the available space minus controls and title
        m_MenuBar->Measure(Size{ availableSize.width - 200.0f, height });
    }
    
    return Size{ availableSize.width, height };
}

void TitleBar::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    
    // Controls
    float controlWidth = 46.0f;
    float currentX = m_Geometry.x + m_Geometry.width - controlWidth * 3.0f;
    
    m_MinimizeRect = Rect{ currentX, m_Geometry.y, controlWidth, m_Geometry.height };
    currentX += controlWidth;
    m_MaximizeRect = Rect{ currentX, m_Geometry.y, controlWidth, m_Geometry.height };
    currentX += controlWidth;
    m_CloseRect = Rect{ currentX, m_Geometry.y, controlWidth, m_Geometry.height };
    
    // Menu Bar
    if (m_MenuBar) {
        float menuWidth = m_MenuBar->GetDesiredSize().width;
        m_MenuBar->Arrange(Rect{ m_Geometry.x + 32.0f, m_Geometry.y, menuWidth, m_Geometry.height });
    }
}

void TitleBar::Paint(PaintContext& context) {
    // Draw background
    context.DrawRect(m_Geometry, Theme::Get().BackgroundDark);
    
    // Draw icon on the far left
    Rect iconRect = { m_Geometry.x + 8.0f, m_Geometry.y + (m_Geometry.height - 16.0f) / 2.0f, 16.0f, 16.0f };
    IconPainter::DrawIcon(context, Icons::Camera, iconRect, Theme::Get().PrimaryAccent); // Engine icon

    // Draw Title centered
    float titleWidth = m_Title.length() * 7.0f; // rough estimate
    Point titlePos = { m_Geometry.x + (m_Geometry.width - titleWidth) / 2.0f, m_Geometry.y + (m_Geometry.height - 13.0f) / 2.0f };
    context.DrawText(m_Title, titlePos, Theme::Get().TextPrimary, 13.0f);
    
    // Child painting (MenuBar)
    for (auto& child : m_Children) {
        child->Paint(context);
    }
    
    // Draw Window Controls
    // Minimize
    if (m_HoveredControl == 0) {
        context.DrawRect(m_MinimizeRect, Theme::Get().HoverOverlay);
    }
    Rect minIconRect = { m_MinimizeRect.x + (m_MinimizeRect.width - 16.0f) / 2.0f, m_MinimizeRect.y + (m_MinimizeRect.height - 16.0f) / 2.0f, 16.0f, 16.0f };
    IconPainter::DrawIcon(context, Icons::Minimize, minIconRect, Theme::Get().TextPrimary);
    
    // Maximize/Restore
    if (m_HoveredControl == 1) {
        context.DrawRect(m_MaximizeRect, Theme::Get().HoverOverlay);
    }
    Rect maxIconRect = { m_MaximizeRect.x + (m_MaximizeRect.width - 16.0f) / 2.0f, m_MaximizeRect.y + (m_MaximizeRect.height - 16.0f) / 2.0f, 16.0f, 16.0f };
    bool isMaximized = (SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MAXIMIZED) != 0;
    IconPainter::DrawIcon(context, isMaximized ? Icons::Restore : Icons::Maximize, maxIconRect, Theme::Get().TextPrimary);
    
    // Close
    if (m_HoveredControl == 2) {
        context.DrawRect(m_CloseRect, Color{ 0.9f, 0.2f, 0.2f, 1.0f }); // Red close button hover
    }
    Rect closeIconRect = { m_CloseRect.x + (m_CloseRect.width - 16.0f) / 2.0f, m_CloseRect.y + (m_CloseRect.height - 16.0f) / 2.0f, 16.0f, 16.0f };
    IconPainter::DrawIcon(context, Icons::X, closeIconRect, Theme::Get().TextPrimary);
}

void TitleBar::OnMouseDown(const MouseEvent& event) {
    if (event.button != MouseButton::Left) return;
    
    if (m_HoveredControl == 0) {
        SDL_MinimizeWindow(m_Window);
    } else if (m_HoveredControl == 1) {
        bool isMaximized = (SDL_GetWindowFlags(m_Window) & SDL_WINDOW_MAXIMIZED) != 0;
        if (isMaximized) {
            SDL_RestoreWindow(m_Window);
        } else {
            SDL_MaximizeWindow(m_Window);
        }
    } else if (m_HoveredControl == 2) {
        SDL_Event quitEvent;
        quitEvent.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quitEvent);
    } else if (m_MenuBar) {
        // Forward to menu bar if in menu bar rect
        if (event.position.x >= m_MenuBar->GetGeometry().x &&
            event.position.x <= m_MenuBar->GetGeometry().x + m_MenuBar->GetGeometry().width) {
            m_MenuBar->OnMouseDown(event);
        }
    }
}

void TitleBar::OnMouseMove(const MouseEvent& event) {
    m_HoveredControl = -1;
    if (event.position.x >= m_MinimizeRect.x && event.position.x <= m_MinimizeRect.x + m_MinimizeRect.width &&
        event.position.y >= m_MinimizeRect.y && event.position.y <= m_MinimizeRect.y + m_MinimizeRect.height) {
        m_HoveredControl = 0;
    } else if (event.position.x >= m_MaximizeRect.x && event.position.x <= m_MaximizeRect.x + m_MaximizeRect.width &&
               event.position.y >= m_MaximizeRect.y && event.position.y <= m_MaximizeRect.y + m_MaximizeRect.height) {
        m_HoveredControl = 1;
    } else if (event.position.x >= m_CloseRect.x && event.position.x <= m_CloseRect.x + m_CloseRect.width &&
               event.position.y >= m_CloseRect.y && event.position.y <= m_CloseRect.y + m_CloseRect.height) {
        m_HoveredControl = 2;
    }
    
    if (m_MenuBar) {
        m_MenuBar->OnMouseMove(event);
    }
}

SDL_HitTestResult TitleBar::HitTest(SDL_Point point) {
    // Determine if the point (in window coordinates) is within title bar draggable area
    // Let's assume the TitleBar is always at y=0 to height
    if (point.y > m_Geometry.height) return SDL_HITTEST_NORMAL;
    
    // Check if in controls
    if (point.x >= m_MinimizeRect.x && point.x <= m_CloseRect.x + m_CloseRect.width) {
        return SDL_HITTEST_NORMAL; // Intercepted by UI controls
    }
    
    // Check if in Menu Bar
    if (m_MenuBar) {
        if (point.x >= m_MenuBar->GetGeometry().x && point.x <= m_MenuBar->GetGeometry().x + m_MenuBar->GetGeometry().width) {
            return SDL_HITTEST_NORMAL; // Let menu bar handle it
        }
    }
    
    // If not in controls, it's draggable
    return SDL_HITTEST_DRAGGABLE;
}

} // namespace HouseEngine::UI
