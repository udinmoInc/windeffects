#include "Label.hpp"
#include "../Core/PaintContext.hpp"

namespace HouseEngine::UI {

Label::Label(const std::string& text, const Color& color, float fontSize)
    : m_Text(text), m_Color(color), m_FontSize(fontSize) {}

Size Label::Measure(const Size& availableSize) {
    (void)availableSize;
    // Estimate size
    float width = static_cast<float>(m_Text.length() * 8);
    float height = m_FontSize + 2.0f;
    m_DesiredSize = Size{ width, height };
    return m_DesiredSize;
}

void Label::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void Label::Paint(PaintContext& context) {
    if (!m_Visible) return;
    context.DrawText(Point{ m_Geometry.x, m_Geometry.y }, m_Text, m_Color, m_FontSize);
}

} // namespace HouseEngine::UI
