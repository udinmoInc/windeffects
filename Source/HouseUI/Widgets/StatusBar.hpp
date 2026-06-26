#pragma once

#include "../Core/Widget.hpp"
#include "../Core/Style.hpp"
#include <string>
#include <vector>
#include <functional>

namespace HouseEngine::UI {

// Status bar section
struct StatusSection {
    std::string text;
    std::string iconName;
    float width = 0.0f; // 0 = auto
    Rect geometry;
};

// Status bar widget for application status information
class StatusBar : public Widget {
public:
    StatusBar();
    virtual ~StatusBar() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    // Section management
    void AddSection(const std::string& text, const std::string& iconName = "", float width = 0.0f);
    void SetSectionText(size_t index, const std::string& text);
    void RemoveSection(size_t index);
    void Clear();

    // Quick access to common sections
    void SetMessage(const std::string& message);
    void SetCoordinates(float x, float y, float z);
    void SetSelectionInfo(size_t count);
    void SetMemoryUsage(size_t usedMB, size_t totalMB);

    // Styling
    void SetHeight(float height) { m_Height = height; }

private:
    void CalculateLayout();

    std::vector<StatusSection> m_Sections;
    float m_Height = 24.0f;
    float m_SectionSpacing = 16.0f;
    float m_Padding = 8.0f;

    WidgetStyle m_Style;
};

} // namespace HouseEngine::UI
