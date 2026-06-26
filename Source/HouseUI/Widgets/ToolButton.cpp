#include "ToolButton.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <algorithm>

namespace HouseEngine::UI {

ToolButton::ToolButton(const std::string& iconName, std::function<void()> onClicked, const std::string& tooltip)
    : m_IconName(iconName)
    , m_Tooltip(tooltip)
    , m_OnClicked(onClicked)
    , m_Style(WidgetStyle::ToolButton())
{}

Size ToolButton::Measure(const Size& availableSize) {
    float iconSize = 20.0f; // Standard icon size
    float padding = Theme::Get().PaddingIconBtn.left + Theme::Get().PaddingIconBtn.right;
    
    return Size{ iconSize + padding, iconSize + padding };
}

void ToolButton::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void ToolButton::Paint(PaintContext& context) {
    // Update animations
    float targetHover = m_Hovered ? 1.0f : 0.0f;
    float targetPress = m_Pressed ? 1.0f : 0.0f;
    float targetActive = m_Active ? 1.0f : 0.0f;
    
    const float animSpeed = 15.0f;
    m_HoverAnim += (targetHover - m_HoverAnim) * animSpeed * 0.016f;
    m_PressAnim += (targetPress - m_PressAnim) * animSpeed * 0.016f;
    m_ActiveAnim += (targetActive - m_ActiveAnim) * animSpeed * 0.016f;
    
    // Determine background color based on state
    Color bgColor = m_Style.background.color;
    if (m_ActiveAnim > 0.01f) {
        bgColor = Color::Lerp(bgColor, Theme::Get().SelectedAccent * 0.3f, m_ActiveAnim);
    }
    if (m_HoverAnim > 0.01f) {
        bgColor = Color::Lerp(bgColor, m_Style.backgroundHover.color, m_HoverAnim);
    }
    if (m_PressAnim > 0.01f) {
        bgColor = Color::Lerp(bgColor, m_Style.backgroundPressed.color, m_PressAnim);
    }
    
    // Draw background with rounded corners
    if (bgColor.a > 0.01f) {
        context.DrawRoundedRect(m_Geometry, bgColor, m_Style.background.cornerRadius);
    }
    
    // Draw icon using Material Icons codepoint
    float iconSize = 18.0f;
    float iconX = m_Geometry.x + (m_Geometry.width - iconSize) / 2.0f;
    float iconY = m_Geometry.y + (m_Geometry.height - iconSize) / 2.0f;
    
    Color iconColor = Theme::Get().TextPrimary;
    if (m_Active) {
        iconColor = Theme::Get().SelectedAccent;
    } else if (m_HoverAnim > 0.1f) {
        iconColor = Color::Lerp(Theme::Get().TextPrimary, Theme::Get().TextSecondary, 0.3f);
    }
    
    int codepoint = Icons::GetCodepoint(m_IconName);
    if (codepoint != 0) {
        context.DrawIcon(codepoint, Point{ iconX, iconY + iconSize }, iconColor, iconSize);
    }
}

void ToolButton::OnMouseDown(const MouseEvent& event) {
    if (event.button == MouseButton::Left) {
        m_Pressed = true;
    }
}

void ToolButton::OnMouseUp(const MouseEvent& event) {
    if (event.button == MouseButton::Left && m_Pressed) {
        m_Pressed = false;
        if (m_OnClicked) {
            m_OnClicked();
        }
    }
}

void ToolButton::OnMouseMove(const MouseEvent& event) {
    // Hover state is handled by EventSystem
}

// ToolSeparator implementation
ToolSeparator::ToolSeparator() {}

Size ToolSeparator::Measure(const Size& availableSize) {
    return Size{ SEPARATOR_WIDTH, SEPARATOR_HEIGHT };
}

void ToolSeparator::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void ToolSeparator::Paint(PaintContext& context) {
    float centerY = m_Geometry.y + m_Geometry.height / 2.0f;
    float halfHeight = SEPARATOR_HEIGHT / 2.0f;
    
    Rect lineRect{ m_Geometry.x, centerY - halfHeight, SEPARATOR_WIDTH, SEPARATOR_HEIGHT };
    context.DrawRect(lineRect, Theme::Get().BorderDefault);
}

} // namespace HouseEngine::UI
