#pragma once

#include "../Core/Widget.hpp"
#include "../Layout/Box.hpp"
#include "../Layout/Spacer.hpp"
#include "../Core/Style.hpp"
#include "../Core/Icon.hpp"
#include "ToolButton.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace HouseEngine::UI {

enum class ToolbarAlignment {
    Left,
    Center,
    Right
};

// Toolbar widget with icon buttons and grouping
class Toolbar : public Widget {
public:
    Toolbar();
    virtual ~Toolbar() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    // Tool management
    std::shared_ptr<ToolButton> AddTool(const std::string& iconName, const std::string& label, std::function<void()> onClick, const std::string& tooltip = "", bool isPlayButton = false, ToolbarAlignment align = ToolbarAlignment::Left);
    void AddSeparator(ToolbarAlignment align = ToolbarAlignment::Left);
    void AddWidget(std::shared_ptr<Widget> widget, ToolbarAlignment align = ToolbarAlignment::Left);
    void Clear();

    // Active tool management
    void SetActiveTool(const std::string& iconName);
    std::string GetActiveTool() const { return m_ActiveTool; }

    // Styling
    void SetHeight(float height) { m_Height = height; }
    void SetIconSize(float size) { m_IconSize = size; }
    void SetFloating(bool floating) { m_IsFloating = floating; }

private:
    struct ToolInfo {
        std::string iconName;
        std::shared_ptr<Widget> button;
        bool isSeparator = false;
    };

    std::vector<ToolInfo> m_Tools;
    std::string m_ActiveTool;
    
    std::shared_ptr<HorizontalBox> m_RootBox;
    std::shared_ptr<HorizontalBox> m_LeftBox;
    std::shared_ptr<HorizontalBox> m_CenterBox;
    std::shared_ptr<HorizontalBox> m_RightBox;

    float m_Height = 30.0f; // Ultra-thin AAA toolbar
    float m_IconSize = 14.0f;
    float m_Spacing = 2.0f;
    bool m_IsFloating = false;

    WidgetStyle m_Style;
};

// Separator for toolbar grouping
class ToolbarSeparator : public Widget {
public:
    ToolbarSeparator();
    virtual ~ToolbarSeparator() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;
};

} // namespace HouseEngine::UI
