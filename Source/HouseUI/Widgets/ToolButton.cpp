#include "ToolButton.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <algorithm>

namespace HouseEngine::UI {

ToolButton::ToolButton(const std::string& iconName, const std::string& label, std::function<void()> onClicked, const std::string& tooltip)
    : m_IconName(iconName)
    , m_Label(label)
    , m_Tooltip(tooltip)
    , m_OnClicked(onClicked)
    , m_Style(WidgetStyle::ToolButton())
{}

Size ToolButton::Measure(const Size& availableSize) {
    if (m_ButtonStyle == ToolButtonStyle::WindowControl || m_ButtonStyle == ToolButtonStyle::WindowClose) {
        m_DesiredSize = Size{ 40.0f, 30.0f }; // Compact hit area for window controls
        return m_DesiredSize;
    }
    
    if (m_ButtonStyle == ToolButtonStyle::TitleBarTool) {
        m_DesiredSize = Size{ 24.0f, 24.0f }; // 24x24 hit area for 14x14 icon
        return m_DesiredSize;
    }

    if (m_ButtonStyle == ToolButtonStyle::ToolbarIconOnly) {
        float width = 26.0f;
        if (m_IsDropdown) width += 16.0f; // 8px chevron + 8px padding
        m_DesiredSize = Size{ width, 26.0f }; // Strict 26px height
        return m_DesiredSize;
    }

    float height = 26.0f; // AAA boxed standard
    float paddingX = 6.0f; 
    float iconSize = 14.0f; // 14px strict icon size
    
    float width = paddingX * 2.0f + iconSize;
    if (!m_Label.empty()) {
        width += 4.0f; // 4px gap between icon and text
        width += m_Label.length() * 7.5f; // Rough text width estimate for 14px text
    }
    
    if (m_IsDropdown) {
        width += 16.0f; // 8px chevron + 8px padding
    }
    
    m_DesiredSize = Size{ width, height };
    return m_DesiredSize;
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
    
    Rect renderRect = m_Geometry;
    
    bool isWindowControl = (m_ButtonStyle == ToolButtonStyle::WindowControl || m_ButtonStyle == ToolButtonStyle::WindowClose);
    bool isTitleBarTool = (m_ButtonStyle == ToolButtonStyle::TitleBarTool);
    
    // Global button scale animation on press (all buttons shrink to 95%)
    if (m_PressAnim > 0.01f && !isWindowControl && !isTitleBarTool) {
        float scale = 1.0f - (m_PressAnim * 0.05f); // Scale to 95%
        float shrinkW = renderRect.width * (1.0f - scale) * 0.5f;
        float shrinkH = renderRect.height * (1.0f - scale) * 0.5f;
        renderRect.x += shrinkW;
        renderRect.y += shrinkH;
        renderRect.width -= shrinkW * 2.0f;
        renderRect.height -= shrinkH * 2.0f;
    }
    
    float iconSize = 24.0f; 
    if (isWindowControl) iconSize = 12.0f;
    if (isTitleBarTool) iconSize = 14.0f;
    
    // For toolbar tools, iconSize is fixed to 14
    bool isToolbarIcon = (m_ButtonStyle == ToolButtonStyle::ToolbarIconOnly);
    bool isPlayButton = (m_ButtonStyle == ToolButtonStyle::PlayButton);
    bool isNormalButton = (m_ButtonStyle == ToolButtonStyle::Normal);
    if (isToolbarIcon || isPlayButton || isNormalButton) iconSize = 14.0f;
    
    float contentWidth = iconSize;
    if (!m_Label.empty()) {
        contentWidth += 4.0f + m_Label.length() * 7.5f;
    }
    
    float currentX = renderRect.x + (renderRect.width - contentWidth) / 2.0f;
    float centerY = renderRect.y + renderRect.height / 2.0f;
    
    if (isWindowControl) {
        if (m_HoverAnim > 0.01f) {
            Color hoverBg = (m_ButtonStyle == ToolButtonStyle::WindowClose) 
                            ? Color{ 0.8f, 0.2f, 0.2f, m_HoverAnim } // Red hover for close
                            : Color{ 0.3f, 0.3f, 0.3f, m_HoverAnim * 0.5f }; // Subtle dark gray
            context.DrawRect(renderRect, hoverBg);
        }
        
        Color iconColor = Color{ 0.72f, 0.72f, 0.72f, 1.0f }; // #B8B8B8 Muted gray
        if (m_ButtonStyle == ToolButtonStyle::WindowClose && m_HoverAnim > 0.5f) {
            iconColor = Color{ 1.0f, 1.0f, 1.0f, 1.0f }; // White when hovering close
        }
        
        int codepoint = Icons::GetCodepoint(m_IconName);
        if (codepoint != 0) {
            context.DrawIcon(codepoint, Point{ currentX, centerY - iconSize/2.0f }, iconColor, iconSize);
        }
    } else {
        if (isToolbarIcon || isPlayButton || isNormalButton) {
            // Uncluttered AAA Desktop Style (UE5 Starship, Rider)
            // Borderless by default. Hover/press get subtle backgrounds.
            // Dropdowns get a thin 1px border.
            
            Color bg = Color{ 0.0f, 0.0f, 0.0f, 0.0f }; // Transparent by default
            bool drawBg = false;

            if (m_PressAnim > 0.01f) {
                // Pressed #1F1F1F
                bg = Color{ 0.12f, 0.12f, 0.12f, 1.0f };
                drawBg = true;
            } else if (m_HoverAnim > 0.01f) {
                // Hover #363636 (Subtle)
                bg = Color{ 0.21f, 0.21f, 0.21f, 0.8f }; // Slight transparency for softer look
                drawBg = true;
            } else if (m_Active) {
                // Active (like a toggled tool)
                bg = Color{ 0.16f, 0.16f, 0.16f, 1.0f };
                drawBg = true;
            }

            if (drawBg) {
                context.DrawRoundedRect(renderRect, bg, 3.0f); // 3px radius for modern tight look
            }
            
            // Dropdowns get a single lightweight container border
            if (m_IsDropdown) {
                context.DrawRoundedRectOutline(renderRect, Color{ 0.25f, 0.25f, 0.25f, 0.8f }, 1.0f, 3.0f);
            }
        }
        
        // Icon color rules:
        // - PlayButton style: ALWAYS blue (#4DA3FF) — the only permanently colored tool
        // - Active (e.g., selected transform): blue
        // - Hovered: white
        // - Default: neutral gray #BFBFBF
        Color iconColor;
        if (isPlayButton) {
            // Play is the ONLY permanently colored icon
            iconColor = Color{ 0.30f, 0.64f, 1.0f, 1.0f }; // #4DA3FF blue always
            if (m_HoverAnim > 0.01f) {
                // Slightly brighter blue on hover
                iconColor = Color{ 0.45f, 0.76f, 1.0f, 1.0f };
            }
            if (m_PressAnim > 0.01f) {
                iconColor = Color{ 0.20f, 0.50f, 0.90f, 1.0f }; // Darker blue on press
            }
        } else {
            iconColor = Color{ 0.75f, 0.75f, 0.75f, 1.0f }; // #BFBFBF neutral gray
            if (m_HoverAnim > 0.01f) {
                iconColor = Color::Lerp(iconColor, Color::White(), m_HoverAnim); // fade to white on hover
            }
            if (m_Active || (m_PressAnim > 0.01f)) {
                iconColor = Color{ 0.30f, 0.64f, 1.0f, 1.0f }; // #4DA3FF when active/pressed
            }
        }
        
        float actualIconSize = 14.0f; // strict 14px
        
        int codepoint = Icons::GetCodepoint(m_IconName);
        if (codepoint != 0) {
            float drawX = currentX;
            if (m_Label.empty()) {
                // Center icon in its designated space
                float spaceWidth = renderRect.width;
                if (m_IsDropdown) spaceWidth -= 16.0f; // Leave space for chevron
                drawX = renderRect.x + (spaceWidth - actualIconSize) / 2.0f;
            } else {
                drawX = renderRect.x + 6.0f; // 6px left padding when there is text
            }
            context.DrawIcon(codepoint, Point{ drawX, centerY - actualIconSize/2.0f }, iconColor, actualIconSize);
            currentX = drawX + actualIconSize + 4.0f;
        }
        
        if (!m_Label.empty()) {
            Color textColor = iconColor;
            context.DrawText(m_Label, Point{ currentX, centerY - 7.0f }, textColor, Theme::Get().TextSizeToolbar);
        }
        
        if (m_IsDropdown) {
            int chevronCodepoint = Icons::GetCodepoint(Icons::ChevronDownName);
            if (chevronCodepoint != 0) {
                float chevronSize = 8.0f;
                float chevronX = renderRect.x + renderRect.width - chevronSize - 8.0f;
                context.DrawIcon(chevronCodepoint, Point{ chevronX, centerY - chevronSize/2.0f }, iconColor, chevronSize);
            }
        }
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
