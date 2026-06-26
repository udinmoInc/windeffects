#pragma once

#include "../Core/Widget.hpp"
#include "../Core/Style.hpp"
#include "../Core/Icon.hpp"
#include "ToolButton.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace HouseEngine::UI {

// Toolbar widget with icon buttons and grouping
class Toolbar : public Widget {
public:
    Toolbar();
    virtual ~Toolbar() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    // Tool management
    void AddTool(const std::string& iconName, std::function<void()> onClick, const std::string& tooltip = "");
    void AddSeparator();
    void AddGroup(const std::vector<std::pair<std::string, std::function<void()>>>& tools);
    void Clear();

    // Active tool management
    void SetActiveTool(const std::string& iconName);
    std::string GetActiveTool() const { return m_ActiveTool; }

    // Styling
    void SetHeight(float height) { m_Height = height; }
    void SetIconSize(float size) { m_IconSize = size; }

private:
    struct ToolInfo {
        std::string iconName;
        std::function<void()> onClick;
        std::string tooltip;
        std::shared_ptr<Widget> button;
        bool isSeparator = false;
    };

    std::vector<ToolInfo> m_Tools;
    std::string m_ActiveTool;
    
    float m_Height = 40.0f;
    float m_IconSize = 20.0f;
    float m_Spacing = 4.0f;
    float m_GroupSpacing = 12.0f;

    WidgetStyle m_Style;
};

// Toolbar group (visual grouping of tools)
class ToolbarGroup : public Widget {
public:
    ToolbarGroup();
    virtual ~ToolbarGroup() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void AddTool(const std::string& iconName, std::function<void()> onClick, const std::string& tooltip = "");
    void Clear();

private:
    std::vector<std::shared_ptr<ToolButton>> m_Tools;
    float m_Spacing = 2.0f;
};

} // namespace HouseEngine::UI
