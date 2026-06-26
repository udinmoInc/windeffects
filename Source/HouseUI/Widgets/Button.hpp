#pragma once

#include "../Core/Widget.hpp"
#include <functional>

namespace HouseEngine::UI {

class Button : public Widget {
public:
    Button(const std::string& labelText, std::function<void()> onClicked = nullptr);
    virtual ~Button() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void OnMouseDown(const MouseEvent& event) override;
    void OnMouseUp(const MouseEvent& event) override;

    void SetText(const std::string& text) { m_Text = text; }
    void SetOnClicked(std::function<void()> onClicked) { m_OnClicked = onClicked; }

private:
    std::string m_Text;
    std::function<void()> m_OnClicked;
    bool m_Pressed = false;
};

} // namespace HouseEngine::UI
