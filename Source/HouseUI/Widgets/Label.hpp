#pragma once

#include "../Core/Widget.hpp"

namespace HouseEngine::UI {

class Label : public Widget {
public:
    Label(const std::string& text, const Color& color = Color::White(), float fontSize = 14.0f);
    virtual ~Label() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void SetText(const std::string& text) { m_Text = text; }
    const std::string& GetText() const { return m_Text; }
    void SetColor(const Color& color) { m_Color = color; }

private:
    std::string m_Text;
    Color m_Color;
    float m_FontSize;
};

} // namespace HouseEngine::UI
