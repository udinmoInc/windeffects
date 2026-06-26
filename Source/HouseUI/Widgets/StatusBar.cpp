#include "StatusBar.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <algorithm>

namespace HouseEngine::UI {

StatusBar::StatusBar()
    : m_Style(WidgetStyle::Panel())
{}

Size StatusBar::Measure(const Size& availableSize) {
    CalculateLayout();
    return Size{ availableSize.width, m_Height };
}

void StatusBar::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    CalculateLayout();
}

void StatusBar::Paint(PaintContext& context) {
    // Draw background
    context.DrawRect(m_Geometry, m_Style.background.color);
    
    // Draw separator line at top
    Rect separatorRect{
        m_Geometry.x,
        m_Geometry.y,
        m_Geometry.width,
        1.0f
    };
    context.DrawRect(separatorRect, Theme::Get().BorderDefault);
    
    // Draw sections
    for (const auto& section : m_Sections) {
        float contentX = section.geometry.x + 4.0f;
        // Draw icon if present
        if (!section.iconName.empty()) {
            float iconSize = 14.0f;
            float iconY = section.geometry.y + (m_Height + iconSize) / 2.0f;
            
            int codepoint = Icons::GetCodepoint(section.iconName);
            if (codepoint != 0) {
                context.DrawIcon(codepoint, Point{ contentX, iconY }, Theme::Get().TextSecondary, iconSize);
            }
            contentX += iconSize + 4.0f;
        }
        
        // Draw text
        float textY = section.geometry.y + (m_Height - 12.0f) / 2.0f;
        
        context.DrawText(section.text, Point{ contentX, textY }, Theme::Get().TextSecondary, 12.0f);
    }
}

void StatusBar::AddSection(const std::string& text, const std::string& iconName, float width) {
    StatusSection section;
    section.text = text;
    section.iconName = iconName;
    section.width = width;
    section.geometry = Rect{};
    m_Sections.push_back(section);
    CalculateLayout();
}

void StatusBar::SetSectionText(size_t index, const std::string& text) {
    if (index < m_Sections.size()) {
        m_Sections[index].text = text;
        CalculateLayout();
    }
}

void StatusBar::RemoveSection(size_t index) {
    if (index < m_Sections.size()) {
        m_Sections.erase(m_Sections.begin() + index);
        CalculateLayout();
    }
}

void StatusBar::Clear() {
    m_Sections.clear();
    CalculateLayout();
}

void StatusBar::SetMessage(const std::string& message) {
    if (m_Sections.empty()) {
        AddSection(message);
    } else {
        SetSectionText(0, message);
    }
}

void StatusBar::SetCoordinates(float x, float y, float z) {
    char coords[64];
    snprintf(coords, sizeof(coords), "X: %.2f Y: %.2f Z: %.2f", x, y, z);
    
    // Find or create coordinates section
    bool found = false;
    for (size_t i = 0; i < m_Sections.size(); ++i) {
        if (m_Sections[i].iconName == Icons::MoveName) {
            SetSectionText(i, coords);
            found = true;
            break;
        }
    }
    
    if (!found) {
        AddSection(coords, Icons::MoveName, 150.0f);
    }
}

void StatusBar::SetSelectionInfo(size_t count) {
    char info[32];
    snprintf(info, sizeof(info), "%zu selected", count);
    
    bool found = false;
    for (size_t i = 0; i < m_Sections.size(); ++i) {
        if (m_Sections[i].iconName == Icons::CursorName) {
            SetSectionText(i, info);
            found = true;
            break;
        }
    }
    
    if (!found) {
        AddSection(info, Icons::CursorName, 100.0f);
    }
}

void StatusBar::SetMemoryUsage(size_t usedMB, size_t totalMB) {
    char mem[32];
    snprintf(mem, sizeof(mem), "Mem: %zu/%zu MB", usedMB, totalMB);
    
    bool found = false;
    for (size_t i = 0; i < m_Sections.size(); ++i) {
        if (m_Sections[i].iconName == Icons::InfoName) {
            SetSectionText(i, mem);
            found = true;
            break;
        }
    }
    
    if (!found) {
        AddSection(mem, Icons::InfoName, 120.0f);
    }
}

void StatusBar::CalculateLayout() {
    float x = m_Geometry.x + m_Padding;
    float availableWidth = m_Geometry.width - m_Padding * 2.0f;
    
    // First pass: calculate total width of fixed sections
    float totalFixedWidth = 0.0f;
    int autoSections = 0;
    
    for (const auto& section : m_Sections) {
        if (section.width > 0.0f) {
            totalFixedWidth += section.width + m_SectionSpacing;
        } else {
            autoSections++;
        }
    }
    
    // Calculate width for auto sections
    float autoSectionWidth = autoSections > 0 ? (availableWidth - totalFixedWidth) / autoSections : 0.0f;
    
    // Second pass: assign geometries
    for (auto& section : m_Sections) {
        float width = section.width > 0.0f ? section.width : autoSectionWidth;
        section.geometry = Rect{ x, m_Geometry.y, width, m_Height };
        x += width + m_SectionSpacing;
    }
}

} // namespace HouseEngine::UI
