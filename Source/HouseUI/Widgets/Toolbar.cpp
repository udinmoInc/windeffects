#include "Toolbar.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include "ToolButton.hpp"
#include <algorithm>

namespace HouseEngine::UI {

Toolbar::Toolbar()
    : m_Style(WidgetStyle::Panel())
{
    m_RootBox = std::make_shared<HorizontalBox>();
    
    m_LeftBox = std::make_shared<HorizontalBox>();
    m_LeftBox->SetSpacing(m_Spacing);

    m_CenterBox = std::make_shared<HorizontalBox>();
    m_CenterBox->SetSpacing(m_Spacing);

    m_RightBox = std::make_shared<HorizontalBox>();
    m_RightBox->SetSpacing(m_Spacing);

    // Build the structure: Left -> Spacer -> Center -> Spacer -> Right
    m_RootBox->AddChild(m_LeftBox);
    m_RootBox->AddChild(std::make_shared<Spacer>());
    m_RootBox->AddChild(m_CenterBox);
    m_RootBox->AddChild(std::make_shared<Spacer>());
    m_RootBox->AddChild(m_RightBox);
}

Size Toolbar::Measure(const Size& availableSize) {
    m_RootBox->Measure(availableSize);
    m_DesiredSize = Size{ availableSize.width, m_Height };
    return m_DesiredSize;
}

void Toolbar::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    
    // Apply internal padding of 8px horizontally
    Rect contentRect{
        allottedRect.x + 8.0f,
        allottedRect.y,
        allottedRect.width - 16.0f,
        allottedRect.height
    };
    
    m_RootBox->Arrange(contentRect);
}

void Toolbar::Paint(PaintContext& context) {
    // Draw background (seamless with editor header)
    context.DrawRect(m_Geometry, Theme::Get().PanelBackground);
    
    // Draw subtle bottom separator instead of heavy border
    Rect separatorRect{
        m_Geometry.x,
        m_Geometry.y + m_Geometry.height - 1.0f,
        m_Geometry.width,
        1.0f
    };
    context.DrawRect(separatorRect, Theme::Get().Separator);
    
    // Paint the root box and its children
    m_RootBox->Paint(context);
}

std::shared_ptr<ToolButton> Toolbar::AddTool(const std::string& iconName, const std::string& label, std::function<void()> onClick, const std::string& tooltip, bool isPlayButton, ToolbarAlignment align) {
    ToolInfo tool;
    tool.iconName = iconName;
    tool.isSeparator = false;
    
    auto btn = std::make_shared<ToolButton>(iconName, label, onClick, tooltip);
    if (isPlayButton) {
        btn->SetButtonStyle(ToolButtonStyle::PlayButton);
    } else {
        // We use ToolbarIconOnly if there's no label for 24x24 bounds
        if (label.empty()) {
            btn->SetButtonStyle(ToolButtonStyle::ToolbarIconOnly);
        }
    }
    tool.button = btn;
    m_Tools.push_back(tool);
    
    AddWidget(btn, align);
    return btn;
}

void Toolbar::AddSeparator(ToolbarAlignment align) {
    ToolInfo tool;
    tool.isSeparator = true;
    auto sep = std::make_shared<ToolbarSeparator>();
    tool.button = sep;
    m_Tools.push_back(tool);
    
    AddWidget(sep, align);
}

void Toolbar::AddWidget(std::shared_ptr<Widget> widget, ToolbarAlignment align) {
    if (align == ToolbarAlignment::Left) {
        m_LeftBox->AddChild(widget);
    } else if (align == ToolbarAlignment::Center) {
        m_CenterBox->AddChild(widget);
    } else if (align == ToolbarAlignment::Right) {
        m_RightBox->AddChild(widget);
    }
}

void Toolbar::Clear() {
    m_Tools.clear();
    m_ActiveTool.clear();
    m_LeftBox->ClearChildren();
    m_CenterBox->ClearChildren();
    m_RightBox->ClearChildren();
}

void Toolbar::SetActiveTool(const std::string& iconName) {
    m_ActiveTool = iconName;
    
    for (auto& tool : m_Tools) {
        if (tool.button && !tool.isSeparator) {
            auto toolBtn = std::dynamic_pointer_cast<ToolButton>(tool.button);
            if (toolBtn) {
                toolBtn->SetActive(tool.iconName == iconName);
            }
        }
    }
}

// ToolbarSeparator implementation
ToolbarSeparator::ToolbarSeparator() {}

Size ToolbarSeparator::Measure(const Size& availableSize) {
    // 12px width + (2px m_Spacing * 2) = 16px total group spacing exactly
    m_DesiredSize = Size{ 12.0f, 16.0f };
    return m_DesiredSize;
}

void ToolbarSeparator::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void ToolbarSeparator::Paint(PaintContext& context) {
    // The user requested: "Increase spacing between logical groups to 16px instead of drawing separators"
    // So we just leave this empty to act as pure whitespace.
}

} // namespace HouseEngine::UI
