#include "Box.hpp"
#include <algorithm>

namespace HouseEngine::UI {

Size Box::Measure(const Size& availableSize) {
    float paddingWidth = m_Padding.left + m_Padding.right;
    float paddingHeight = m_Padding.top + m_Padding.bottom;

    Size contentAvailable{
        std::max(0.0f, availableSize.width - paddingWidth),
        std::max(0.0f, availableSize.height - paddingHeight)
    };

    float totalWidth = 0.0f;
    float totalHeight = 0.0f;
    bool first = true;

    for (const auto& child : m_Children) {
        if (!child->IsVisible()) continue;

        Size childDesired = child->Measure(contentAvailable);

        if (m_Orientation == Orientation::Horizontal) {
            totalWidth += childDesired.width;
            if (!first) totalWidth += m_Spacing;
            totalHeight = std::max(totalHeight, childDesired.height);
        } else {
            totalHeight += childDesired.height;
            if (!first) totalHeight += m_Spacing;
            totalWidth = std::max(totalWidth, childDesired.width);
        }
        first = false;
    }

    m_DesiredSize = Size{
        totalWidth + paddingWidth,
        totalHeight + paddingHeight
    };

    return m_DesiredSize;
}

void Box::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;

    float paddingWidth = m_Padding.left + m_Padding.right;
    float paddingHeight = m_Padding.top + m_Padding.bottom;
    float contentWidth = std::max(0.0f, allottedRect.width - paddingWidth);
    float contentHeight = std::max(0.0f, allottedRect.height - paddingHeight);

    float currentX = allottedRect.x + m_Padding.left;
    float currentY = allottedRect.y + m_Padding.top;
    bool first = true;

    // Count visible children
    int visibleCount = 0;
    for (const auto& child : m_Children) {
        if (child->IsVisible()) visibleCount++;
    }

    // We can distribute extra space if children want to stretch, but for v0.1
    // let's do direct sequential layout using desired sizes.
    for (size_t i = 0; i < m_Children.size(); ++i) {
        const auto& child = m_Children[i];
        if (!child->IsVisible()) continue;

        Size childDesired = child->GetDesiredSize();
        bool isLast = (i == m_Children.size() - 1);

        if (m_Orientation == Orientation::Horizontal) {
            if (!first) currentX += m_Spacing;
            
            float childWidth = isLast ? (allottedRect.x + allottedRect.width - currentX) : childDesired.width;
            float childHeight = std::min(contentHeight, childDesired.height);
            float childY = currentY + (contentHeight - childHeight) * 0.5f;

            child->Arrange(Rect{ currentX, childY, childWidth, childHeight });
            currentX += childWidth;
        } else {
            if (!first) currentY += m_Spacing;

            float childWidth = std::min(contentWidth, childDesired.width);
            float childHeight = isLast ? (allottedRect.y + allottedRect.height - currentY) : childDesired.height;
            float childX = currentX + (contentWidth - childWidth) * 0.5f;

            child->Arrange(Rect{ childX, currentY, childWidth, childHeight });
            currentY += childHeight;
        }
        first = false;
    }
}

void Box::Paint(PaintContext& context) {
    if (!m_Visible) return;
    for (const auto& child : m_Children) {
        if (child->IsVisible()) {
            child->Paint(context);
        }
    }
}

} // namespace HouseEngine::UI
