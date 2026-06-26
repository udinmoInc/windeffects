#include "PaintContext.hpp"

namespace HouseEngine::UI {

void PaintContext::PushClipRect(const Rect& clip) {
    if (m_ClipStack.empty()) {
        m_ClipStack.push_back(clip);
    } else {
        // Intersect with parent clip rect
        Rect parentClip = m_ClipStack.back();
        m_ClipStack.push_back(parentClip.Intersect(clip));
    }
}

void PaintContext::PopClipRect() {
    if (!m_ClipStack.empty()) {
        m_ClipStack.pop_back();
    }
}

Rect PaintContext::GetCurrentClipRect() const {
    if (m_ClipStack.empty()) {
        // Return a massive rect representing no clipping
        return { 0.0f, 0.0f, 100000.0f, 100000.0f };
    }
    return m_ClipStack.back();
}

void PaintContext::DrawRect(const Rect& rect, const Color& color, float borderRadius) {
    DrawCommand cmd{};
    cmd.type = DrawCommandType::Rect;
    cmd.rect = rect;
    cmd.color = color;
    cmd.clipRect = GetCurrentClipRect();
    cmd.borderRadius = borderRadius;
    m_Commands.push_back(cmd);
}

void PaintContext::DrawText(const Point& pos, const std::string& text, const Color& color, float fontSize) {
    DrawCommand cmd{};
    cmd.type = DrawCommandType::Text;
    // Store position in rect.x, rect.y
    cmd.rect = { pos.x, pos.y, 0.0f, 0.0f };
    cmd.color = color;
    cmd.clipRect = GetCurrentClipRect();
    cmd.text = text;
    cmd.fontSize = fontSize;
    m_Commands.push_back(cmd);
}

void PaintContext::DrawLine(const Point& start, const Point& end, const Color& color, float thickness) {
    DrawCommand cmd{};
    cmd.type = DrawCommandType::Line;
    cmd.color = color;
    cmd.clipRect = GetCurrentClipRect();
    cmd.lineStart = start;
    cmd.lineEnd = end;
    cmd.thickness = thickness;
    m_Commands.push_back(cmd);
}

void PaintContext::DrawTexture(const Rect& rect, VkDescriptorSet textureId) {
    DrawCommand cmd{};
    cmd.type = DrawCommandType::Texture;
    cmd.rect = rect;
    cmd.color = Color::White(); // Default tint
    cmd.clipRect = GetCurrentClipRect();
    cmd.textureId = textureId;
    m_Commands.push_back(cmd);
}

} // namespace HouseEngine::UI
