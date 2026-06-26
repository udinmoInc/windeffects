#include "TextBox.hpp"
#include "../Core/PaintContext.hpp"

namespace HouseEngine::UI {

TextBox::TextBox(const std::string& initialText, std::function<void(const std::string&)> onTextChanged)
    : m_Text(initialText), m_OnTextChanged(onTextChanged) {}

Size TextBox::Measure(const Size& availableSize) {
    (void)availableSize;
    // Default textbox size
    m_DesiredSize = Size{ 150.0f, 22.0f };
    return m_DesiredSize;
}

void TextBox::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
}

void TextBox::Paint(PaintContext& context) {
    if (!m_Visible) return;

    // Dark input field color
    Color bgColor = Color{ 0.08f, 0.08f, 0.10f, 1.0f };
    Color borderColor = m_Focused ? Color{ 0.20f, 0.40f, 0.70f, 1.0f } : Color{ 0.20f, 0.20f, 0.22f, 1.0f };

    // 1. Draw background
    context.DrawRect(m_Geometry, bgColor, 2.0f);

    // 2. Draw border
    context.DrawLine(Point{m_Geometry.x, m_Geometry.y}, Point{m_Geometry.x + m_Geometry.width, m_Geometry.y}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x + m_Geometry.width, m_Geometry.y}, Point{m_Geometry.x + m_Geometry.width, m_Geometry.y + m_Geometry.height}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x + m_Geometry.width, m_Geometry.y + m_Geometry.height}, Point{m_Geometry.x, m_Geometry.y + m_Geometry.height}, borderColor, 1.0f);
    context.DrawLine(Point{m_Geometry.x, m_Geometry.y + m_Geometry.height}, Point{m_Geometry.x, m_Geometry.y}, borderColor, 1.0f);

    // 3. Draw text offset slightly
    float textX = m_Geometry.x + 6.0f;
    float textY = m_Geometry.y + (m_Geometry.height - 12.0f) * 0.5f;
    context.DrawText(Point{ textX, textY }, m_Text, Color::White(), 14.0f);

    // 4. Draw blinking/static cursor if focused
    if (m_Focused) {
        float cursorX = textX + static_cast<float>(m_Text.length() * 8) + 2.0f;
        float cursorY = textY;
        context.DrawLine(Point{ cursorX, cursorY }, Point{ cursorX, cursorY + 12.0f }, Color{ 0.30f, 0.60f, 0.90f, 1.0f }, 1.5f);
    }
}

void TextBox::OnMouseDown(const MouseEvent& event) {
    (void)event;
    // Handled by EventSystem focus router
}

void TextBox::OnKeyDown(const KeyEvent& event) {
    if (!m_Focused) return;

    bool changed = false;

    if (event.keycode == SDLK_BACKSPACE) {
        if (!m_Text.empty()) {
            m_Text.pop_back();
            changed = true;
        }
    } else if (event.keycode == SDLK_RETURN || event.keycode == SDLK_ESCAPE) {
        // Blur text box on commit/cancel
        m_Focused = false;
    } else if (event.keycode == SDLK_SPACE) {
        m_Text += ' ';
        changed = true;
    } else {
        char typedChar = 0;
        // Map alphabetical keys
        if (event.keycode >= 'a' && event.keycode <= 'z') {
            if (event.shiftDown) {
                typedChar = static_cast<char>(event.keycode - 32); // Uppercase
            } else {
                typedChar = static_cast<char>(event.keycode); // Lowercase
            }
        }
        // Map digits
        else if (event.keycode >= '0' && event.keycode <= '9') {
            typedChar = static_cast<char>(event.keycode);
        }
        // Basic symbols
        else if (event.keycode == '-' || event.keycode == '_' || event.keycode == '.' || event.keycode == ',') {
            typedChar = static_cast<char>(event.keycode);
        }

        if (typedChar != 0 && m_Text.length() < 32) { // Limit length
            m_Text += typedChar;
            changed = true;
        }
    }

    if (changed && m_OnTextChanged) {
        m_OnTextChanged(m_Text);
    }
}

} // namespace HouseEngine::UI
