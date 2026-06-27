#include "TabWidget.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <algorithm>

namespace HouseEngine::UI {

TabWidget::TabWidget()
    : m_TabStyle(WidgetStyle::Tab())
    , m_ActiveTabStyle(WidgetStyle::TabActive())
{}

Size TabWidget::Measure(const Size& availableSize) {
    CalculateTabGeometries();
    
    // Measure content of active tab
    if (m_ActiveTab >= 0 && m_ActiveTab < static_cast<int>(m_Tabs.size())) {
        auto& tab = m_Tabs[m_ActiveTab];
        if (tab.content) {
            Size contentSize = tab.content->Measure(availableSize);
            return Size{ availableSize.width, m_TabHeight + contentSize.height };
        }
    }
    
    return Size{ availableSize.width, m_TabHeight + 100.0f };
}

void TabWidget::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    CalculateTabGeometries();
    
    // Arrange content of active tab
    if (m_ActiveTab >= 0 && m_ActiveTab < static_cast<int>(m_Tabs.size())) {
        auto& tab = m_Tabs[m_ActiveTab];
        if (tab.content) {
            Rect contentRect{
                allottedRect.x,
                allottedRect.y + m_TabHeight,
                allottedRect.width,
                allottedRect.height - m_TabHeight
            };
            tab.content->Arrange(contentRect);
        }
    }
}

void TabWidget::Paint(PaintContext& context) {
    // Flat dark background for the tab strip (Title bar color)
    Color stripBg = Theme::Get().TabBackground;
    Rect tabBarRect{ m_Geometry.x, m_Geometry.y, m_Geometry.width, m_TabHeight };
    context.DrawRect(tabBarRect, stripBg);
    
    // No separator line in modern AAA unless necessary, but we can draw a 1px baseline
    Rect separatorRect{
        m_Geometry.x,
        m_Geometry.y + m_TabHeight - 1.0f,
        m_Geometry.width,
        1.0f
    };
    context.DrawRect(separatorRect, Theme::Get().BorderDefault);
    
    // Draw tabs
    for (size_t i = 0; i < m_Tabs.size(); ++i) {
        const auto& tab = m_Tabs[i];
        bool isActive = (static_cast<int>(i) == m_ActiveTab);
        bool isHovered = (static_cast<int>(i) == m_HoveredTab);
        
        // Draw tab background
        if (isActive) {
            // Match the viewport background so it connects seamlessly
            Color activeBg = Theme::Get().ViewportBackground;
            // Draw slightly taller so it covers the separator line below it
            Rect activeTabRect = tab.geometry;
            activeTabRect.height += 1.0f; // Cover the 1px separator
            context.DrawRect(activeTabRect, activeBg);
            
            // Subtle accent line (2px blue underline) at the bottom
            Rect underlineRect{ tab.geometry.x, tab.geometry.y + m_TabHeight - 2.0f, tab.geometry.width, 2.0f };
            context.DrawRect(underlineRect, Theme::Get().SelectedAccent);
        } else if (isHovered) {
            // Very subtle hover on inactive tabs
            Color hoverBg = Color{ 1.0f, 1.0f, 1.0f, 0.05f };
            context.DrawRect(tab.geometry, hoverBg);
        }
        
        // Draw tab label
        float textX = tab.geometry.x + 14.0f; // 14px padding
        float textY = tab.geometry.y + (m_TabHeight - Theme::Get().TextSizeTabs) / 2.0f;
        
        Color textColor = isActive ? Color::White() : Theme::Get().TextSecondary;
        if (isHovered && !isActive) textColor = Theme::Get().TextPrimary;
        
        context.DrawText(tab.label, Point{ textX, textY }, textColor, Theme::Get().TextSizeTabs);
        // Simulate bold text for active tab by drawing it again with a small offset
        if (isActive) {
            context.DrawText(tab.label, Point{ textX + 0.5f, textY }, textColor, Theme::Get().TextSizeTabs);
        }
        
        // Draw close button if active or hovered
        if (isActive || isHovered) {
            float textWidth = tab.label.length() * 7.5f;
            float iconSize = 12.0f;
            float iconX = tab.geometry.x + 14.0f + textWidth + 8.0f; // 8px space after text
            
            Color iconColor = Color{ 0.5f, 0.5f, 0.5f, 1.0f };
            if (isActive) iconColor = Color{ 0.7f, 0.7f, 0.7f, 1.0f };
            
            int codepoint = Icons::GetCodepoint("x");
            if (codepoint != 0) {
                context.DrawIcon(codepoint, Point{ iconX, tab.geometry.y + (m_TabHeight - iconSize) / 2.0f }, iconColor, iconSize);
            }
        }
    }
    
    // Draw active tab content
    if (m_ActiveTab >= 0 && m_ActiveTab < static_cast<int>(m_Tabs.size())) {
        auto& tab = m_Tabs[m_ActiveTab];
        if (tab.content) {
            tab.content->Paint(context);
        }
    }
}

void TabWidget::OnMouseDown(const MouseEvent& event) {
    for (size_t i = 0; i < m_Tabs.size(); ++i) {
        const auto& tab = m_Tabs[i];
        if (event.position.x >= tab.geometry.x && event.position.x <= tab.geometry.x + tab.geometry.width &&
            event.position.y >= tab.geometry.y && event.position.y <= tab.geometry.y + tab.geometry.height) {
            SetActiveTab(static_cast<int>(i));
            return;
        }
    }
}

void TabWidget::OnMouseMove(const MouseEvent& event) {
    m_HoveredTab = -1;
    
    for (size_t i = 0; i < m_Tabs.size(); ++i) {
        const auto& tab = m_Tabs[i];
        if (event.position.x >= tab.geometry.x && event.position.x <= tab.geometry.x + tab.geometry.width &&
            event.position.y >= tab.geometry.y && event.position.y <= tab.geometry.y + tab.geometry.height) {
            m_HoveredTab = static_cast<int>(i);
            return;
        }
    }
}

void TabWidget::AddTab(const std::string& label) {
    TabInfo tab;
    tab.label = label;
    m_Tabs.push_back(tab);
    
    if (m_ActiveTab == -1) {
        SetActiveTab(0);
    }
    
    CalculateTabGeometries();
}

void TabWidget::RemoveTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_Tabs.size())) return;
    
    m_Tabs.erase(m_Tabs.begin() + index);
    
    if (m_ActiveTab == index) {
        m_ActiveTab = std::min(m_ActiveTab, static_cast<int>(m_Tabs.size()) - 1);
    } else if (m_ActiveTab > index) {
        m_ActiveTab--;
    }
    
    CalculateTabGeometries();
}

void TabWidget::SetTabLabel(int index, const std::string& label) {
    if (index >= 0 && index < static_cast<int>(m_Tabs.size())) {
        m_Tabs[index].label = label;
        CalculateTabGeometries();
    }
}

void TabWidget::SetActiveTab(int index) {
    if (index == m_ActiveTab) return;
    if (index < 0 || index >= static_cast<int>(m_Tabs.size())) return;
    
    m_ActiveTab = index;
    
    if (m_OnTabChanged) {
        m_OnTabChanged(index);
    }
    
    // Re-arrange to show new content
    Arrange(m_Geometry);
}

void TabWidget::SetTabContent(int index, const std::shared_ptr<Widget>& content) {
    if (index >= 0 && index < static_cast<int>(m_Tabs.size())) {
        m_Tabs[index].content = content;
    }
}

std::shared_ptr<Widget> TabWidget::GetTabContent(int index) const {
    if (index >= 0 && index < static_cast<int>(m_Tabs.size())) {
        return m_Tabs[index].content;
    }
    return nullptr;
}

void TabWidget::CalculateTabGeometries() {
    float x = m_Geometry.x;
    
    if (m_Tabs.empty()) return;
    
    for (size_t i = 0; i < m_Tabs.size(); ++i) {
        auto& tab = m_Tabs[i];
        bool isActive = (static_cast<int>(i) == m_ActiveTab);
        bool isHovered = (static_cast<int>(i) == m_HoveredTab);
        
        // AAA Desktop padding: 14px on left, 14px on right.
        float padding = 28.0f; 
        
        // Add space for close button if active or hovered
        if (isActive || isHovered) {
            padding += 16.0f; // 12px icon + 4px spacing
        }
        
        float textWidth = tab.label.length() * 7.5f; // Rough text width estimate
        float tabWidth = std::max(m_TabMinWidth, textWidth + padding);
        
        tab.geometry = Rect{ x, m_Geometry.y, tabWidth, m_TabHeight };
        x += tabWidth + m_TabSpacing;
    }
}

} // namespace HouseEngine::UI
