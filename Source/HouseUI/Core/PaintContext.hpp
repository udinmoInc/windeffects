#pragma once

#include <vector>
#include <string>
#include "Geometry.hpp"
#include <volk.h>

namespace HouseEngine::UI {

enum class DrawCommandType {
    Rect,
    Text,
    Line,
    Texture
};

struct DrawCommand {
    DrawCommandType type;
    Rect rect;
    Color color;
    Rect clipRect;      // Scissor clipping
    VkDescriptorSet textureId = VK_NULL_HANDLE; // Used for viewport or icons
    std::string text;
    float fontSize = 14.0f;
    float borderRadius = 0.0f;
    float thickness = 1.0f;
    Point lineStart;
    Point lineEnd;
};

class PaintContext {
public:
    void PushClipRect(const Rect& clip);
    void PopClipRect();

    void DrawRect(const Rect& rect, const Color& color, float borderRadius = 0.0f);
    void DrawText(const Point& pos, const std::string& text, const Color& color, float fontSize = 14.0f);
    void DrawLine(const Point& start, const Point& end, const Color& color, float thickness = 1.0f);
    void DrawTexture(const Rect& rect, VkDescriptorSet textureId);

    const std::vector<DrawCommand>& GetCommands() const { return m_Commands; }
    void Clear() { m_Commands.clear(); m_ClipStack.clear(); }

private:
    std::vector<DrawCommand> m_Commands;
    std::vector<Rect> m_ClipStack;
    Rect GetCurrentClipRect() const;
};

} // namespace HouseEngine::UI
