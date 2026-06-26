#include "Button.hpp"
#include "../Core/PaintContext.hpp"

namespace HouseEngine::UI {

Button::Button(const std::string& labelText, std::function<void()> onClicked)
    : m_Text(labelText), m_OnClicked(onClicked) {}

Size Button::Measure(const Size& availableSize) {
    (void)availableSize;
    // Simple width estimation: ~8 pixels per character plus padding
    float width = static_cast<float>(m_Text.length() * 8 + 16);
    float height = 22.0f; // Standard button height
    m_DesiredSize = Size{ width, height };
    return m_DesiredSize;
}

void Button::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void Button::Paint(PaintContext& context) {
    if (!m_Visible) return;

    // Determine colors based on interaction state
    Color bgColor;
    Color textColor = Color::White();

    if (m_Pressed) {
        bgColor = Color{ 0.08f, 0.20f, 0.38f, 1.0f }; // Slate blue pressed
    } else if (m_Hovered) {
        bgColor = Color{ 0.28f, 0.30f, 0.33f, 1.0f }; // Lighter charcoal hovered
    } else {
        bgColor = Color{ 0.18f, 0.18f, 0.20f, 1.0f }; // Standard slate gray
    }

    // 1. Draw button background
    context.DrawRect(m_Geometry, bgColor, 3.0f);

    // 2. Draw border
    Color borderColor = m_Hovered ? Color{ 0.35f, 0.45f, 0.60f, 1.0f } : Color{ 0.24f, 0.24f, 0.26f, 1.0f };
    // Draw outline using fine rectangle lines
    context.DrawLine(Point{m_Geometry.x, m_Geometry.y}, Point{m_Geometry.x + m_Geometry.width, m_Geometry.y}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x + m_Geometry.width, m_Geometry.y}, Point{m_Geometry.x + m_Geometry.width, m_Geometry.y + m_Geometry.height}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x + m_Geometry.width, m_Geometry.y + m_Geometry.height}, Point{m_Geometry.x, m_Geometry.y + m_Geometry.height}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x, m_Geometry.y + m_Geometry.height}, Point{m_Geometry.x, m_Geometry.y}, borderColor, 1.0f);

    // 3. Draw text centered
    float textW = static_cast<float>(m_Text.length() * 8);
    float textX = m_Geometry.x + (m_Geometry.width - textW) * 0.5f;
    float textY = m_Geometry.y + (m_Geometry.height - 12.0f) * 0.5f; // Adjust vertically
    context.DrawText(Point{ textX, textY }, m_Text, textColor, 14.0f);
}

void Button::OnMouseDown(const MouseEvent& event) {
    if (event.button == MouseButton::Left) {
        m_Pressed = true;
    }
}

void Button::OnMouseUp(const MouseEvent& event) {
    if (event.button == MouseButton::Left && m_Pressed) {
        m_Pressed = false;
        if (m_Geometry.Contains(event.position) && m_OnClicked) {
            m_OnClicked();
        }
    }
}

} // namespace HouseEngine::UI
