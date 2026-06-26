#include "ContentBrowser.hpp"
#include "../Core/PaintContext.hpp"
#include "../Core/Theme.hpp"
#include "../Core/Icon.hpp"
#include <algorithm>

namespace HouseEngine::UI {

ContentBrowser::ContentBrowser()
    : m_Style(WidgetStyle::Panel())
{}

Size ContentBrowser::Measure(const Size& availableSize) {
    BuildRenderList();
    
    if (m_ViewMode == ContentViewMode::Grid) {
        CalculateGridLayout();
    } else {
        CalculateListLayout();
    }
    
    float totalHeight = 0.0f;
    for (const auto& item : m_RenderList) {
        totalHeight = std::max(totalHeight, item.geometry.y + item.geometry.height - m_Geometry.y);
    }
    
    return Size{ availableSize.width, totalHeight };
}

void ContentBrowser::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    
    if (m_ViewMode == ContentViewMode::Grid) {
        CalculateGridLayout();
    } else {
        CalculateListLayout();
    }
}

void ContentBrowser::Paint(PaintContext& context) {
    // Draw background
    context.DrawRect(m_Geometry, Theme::Get().PanelBackground);
    
    // Draw items
    for (const auto& renderItem : m_RenderList) {
        const auto& item = renderItem.item;
        
        // Skip if outside visible area
        if (renderItem.geometry.y + renderItem.geometry.height < m_Geometry.y ||
            renderItem.geometry.y > m_Geometry.y + m_Geometry.height) {
            continue;
        }
        
        // Draw selection background
        if (item.id == m_SelectedId) {
            context.DrawRoundedRect(renderItem.geometry, Theme::Get().SelectedAccent * 0.3f, 4.0f);
        } else if (item.id == m_HoveredId) {
            context.DrawRoundedRect(renderItem.geometry, Theme::Get().HoverOverlay, 4.0f);
        }
        
        if (m_ViewMode == ContentViewMode::Grid) {
            // Draw thumbnail/icon
            float iconSize = m_ThumbnailSize;
            float iconX = renderItem.geometry.x + (m_GridItemSize - iconSize) / 2.0f;
            float iconY = renderItem.geometry.y + 8.0f;
            
            Rect iconRect{ iconX, iconY, iconSize, iconSize };
            IconPainter::DrawIcon(context, item.iconName, iconRect, Theme::Get().TextPrimary);
            
            // Draw label
            float labelWidth = m_GridItemSize - 8.0f;
            float labelX = renderItem.geometry.x + 4.0f;
            float labelY = renderItem.geometry.y + iconSize + 12.0f;
            
            // Truncate text if too long
            std::string displayName = item.name;
            if (displayName.length() > 12) {
                displayName = displayName.substr(0, 10) + "...";
            }
            
            context.DrawText(displayName, Point{ labelX, labelY }, Theme::Get().TextPrimary, 11.0f);
            
            // Draw favorite star
            if (item.isFavorite) {
                float starSize = 14.0f;
                float starX = renderItem.geometry.x + renderItem.geometry.width - starSize - 4.0f;
                float starY = renderItem.geometry.y + 4.0f;
                Rect starRect{ starX, starY, starSize, starSize };
                IconPainter::DrawIcon(context, Icons::StarFilled, starRect, Theme::Get().SelectedAccent);
            }
        } else {
            // List view
            float iconSize = 20.0f;
            float iconX = renderItem.geometry.x + 8.0f;
            float iconY = renderItem.geometry.y + (m_ListRowHeight - iconSize) / 2.0f;
            
            Rect iconRect{ iconX, iconY, iconSize, iconSize };
            IconPainter::DrawIcon(context, item.iconName, iconRect, Theme::Get().TextPrimary);
            
            // Draw name
            float nameX = iconX + iconSize + 8.0f;
            float nameY = renderItem.geometry.y + (m_ListRowHeight - 13.0f) / 2.0f;
            context.DrawText(item.name, Point{ nameX, nameY }, Theme::Get().TextPrimary, 13.0f);
            
            // Draw type
            float typeX = renderItem.geometry.x + renderItem.geometry.width - item.type.length() * 12.0f * 0.6f - 8.0f;
            float typeY = renderItem.geometry.y + (m_ListRowHeight - 12.0f) / 2.0f;
            context.DrawText(item.type, Point{ typeX, typeY }, Theme::Get().TextSecondary, 12.0f);
        }
    }
}

void ContentBrowser::OnMouseDown(const MouseEvent& event) {
    RenderItem* renderItem = GetItemAtPosition(event.position);
    if (renderItem) {
        SetSelectedId(renderItem->item.id);
        
        if (m_OnItemSelected) {
            m_OnItemSelected(renderItem->item);
        }
    } else {
        ClearSelection();
    }
}

void ContentBrowser::OnMouseMove(const MouseEvent& event) {
    RenderItem* renderItem = GetItemAtPosition(event.position);
    m_HoveredId = renderItem ? renderItem->item.id : "";
}

void ContentBrowser::OnMouseWheel(const MouseEvent& event) {
    float scrollAmount = event.wheelDeltaY * (m_ViewMode == ContentViewMode::Grid ? m_GridItemSize : m_ListRowHeight) * 0.5f;
    m_ScrollOffset -= scrollAmount;
    
    // Clamp scroll
    float totalHeight = Measure(Size{m_Geometry.width, m_Geometry.height}).height;
    float maxScroll = std::max(0.0f, totalHeight - m_Geometry.height);
    m_ScrollOffset = std::max(0.0f, std::min(m_ScrollOffset, maxScroll));
    
    Arrange(m_Geometry);
}

void ContentBrowser::OnKeyDown(const KeyEvent& event) {
    // TODO: Implement keyboard navigation
}

void ContentBrowser::AddItem(const ContentItem& item) {
    m_Items.push_back(item);
    BuildRenderList();
}

void ContentBrowser::RemoveItem(const std::string& id) {
    m_Items.erase(
        std::remove_if(m_Items.begin(), m_Items.end(),
            [&id](const ContentItem& item) { return item.id == id; }),
        m_Items.end()
    );
    BuildRenderList();
}

void ContentBrowser::Clear() {
    m_Items.clear();
    m_RenderList.clear();
    m_SelectedId.clear();
    m_HoveredId.clear();
    m_ScrollOffset = 0.0f;
}

void ContentBrowser::SetSelectedId(const std::string& id) {
    m_SelectedId = id;
}

void ContentBrowser::BuildRenderList() {
    m_RenderList.clear();
    
    for (const auto& item : m_Items) {
        RenderItem renderItem;
        renderItem.item = item;
        renderItem.geometry = Rect{};
        m_RenderList.push_back(renderItem);
    }
}

void ContentBrowser::CalculateGridLayout() {
    float x = m_Geometry.x + 8.0f;
    float y = m_Geometry.y + 8.0f - m_ScrollOffset;
    int itemsPerRow = std::max(1, static_cast<int>((m_Geometry.width - 16.0f) / (m_GridItemSize + m_GridSpacing)));
    
    for (size_t i = 0; i < m_RenderList.size(); ++i) {
        int col = static_cast<int>(i) % itemsPerRow;
        int row = static_cast<int>(i) / itemsPerRow;
        
        m_RenderList[i].geometry = Rect{
            x + col * (m_GridItemSize + m_GridSpacing),
            y + row * (m_GridItemSize + m_GridSpacing + 20.0f), // +20 for label
            m_GridItemSize,
            m_GridItemSize + 20.0f
        };
    }
}

void ContentBrowser::CalculateListLayout() {
    float y = m_Geometry.y - m_ScrollOffset;
    
    for (auto& renderItem : m_RenderList) {
        renderItem.geometry = Rect{
            m_Geometry.x,
            y,
            m_Geometry.width,
            m_ListRowHeight
        };
        y += m_ListRowHeight;
    }
}

ContentBrowser::RenderItem* ContentBrowser::GetItemAtPosition(const Point& pos) {
    for (auto& renderItem : m_RenderList) {
        if (pos.x >= renderItem.geometry.x && pos.x <= renderItem.geometry.x + renderItem.geometry.width &&
            pos.y >= renderItem.geometry.y && pos.y <= renderItem.geometry.y + renderItem.geometry.height) {
            return &renderItem;
        }
    }
    return nullptr;
}

// Breadcrumb implementation
Breadcrumb::Breadcrumb() {}

Size Breadcrumb::Measure(const Size& availableSize) {
    CalculateLayout();
    
    float totalWidth = 0.0f;
    for (const auto& crumb : m_Crumbs) {
        totalWidth += crumb.geometry.width;
    }
    
    return Size{ totalWidth, 24.0f };
}

void Breadcrumb::Arrange(const Rect& allottedRect) {
    m_Geometry = allottedRect;
    CalculateLayout();
}

void Breadcrumb::Paint(PaintContext& context) {
    for (size_t i = 0; i < m_Crumbs.size(); ++i) {
        const auto& crumb = m_Crumbs[i];
        
        // Draw hover background
        if (crumb.hovered) {
            context.DrawRoundedRect(crumb.geometry, Theme::Get().HoverOverlay, 4.0f);
        }
        
        // Draw crumb text
        float textX = crumb.geometry.x + 8.0f;
        float textY = crumb.geometry.y + (24.0f - 13.0f) / 2.0f;
        
        Color textColor = (static_cast<int>(i) == m_HoveredCrumb) ? Theme::Get().SelectedAccent : Theme::Get().TextPrimary;
        context.DrawText(crumb.text, Point{ textX, textY }, textColor, 13.0f);
        
        // Draw separator (chevron)
        if (i < m_Crumbs.size() - 1) {
            float sepX = crumb.geometry.x + crumb.geometry.width + m_CrumbSpacing;
            float sepY = crumb.geometry.y + (24.0f - 12.0f) / 2.0f;
            Rect sepRect{ sepX, sepY, 12.0f, 12.0f };
            IconPainter::DrawIcon(context, Icons::ChevronRight, sepRect, Theme::Get().TextSecondary);
        }
    }
}

void Breadcrumb::OnMouseDown(const MouseEvent& event) {
    CrumbInfo* crumb = GetCrumbAtPosition(event.position);
    if (crumb && m_OnCrumbClicked) {
        size_t index = crumb - &m_Crumbs[0];
        m_OnCrumbClicked(index);
    }
}

void Breadcrumb::OnMouseMove(const MouseEvent& event) {
    m_HoveredCrumb = -1;
    
    CrumbInfo* crumb = GetCrumbAtPosition(event.position);
    if (crumb) {
        m_HoveredCrumb = static_cast<int>(crumb - &m_Crumbs[0]);
    }
    
    for (auto& c : m_Crumbs) {
        c.hovered = false;
    }
    
    if (m_HoveredCrumb >= 0) {
        m_Crumbs[m_HoveredCrumb].hovered = true;
    }
}

void Breadcrumb::SetPath(const std::vector<std::string>& path) {
    m_Crumbs.clear();
    
    for (const auto& crumb : path) {
        CrumbInfo info;
        info.text = crumb;
        info.geometry = Rect{};
        m_Crumbs.push_back(info);
    }
    
    CalculateLayout();
}

void Breadcrumb::AddCrumb(const std::string& crumb) {
    CrumbInfo info;
    info.text = crumb;
    info.geometry = Rect{};
    m_Crumbs.push_back(info);
    CalculateLayout();
}

void Breadcrumb::Clear() {
    m_Crumbs.clear();
}

void Breadcrumb::CalculateLayout() {
    float x = m_Geometry.x;
    
    for (auto& crumb : m_Crumbs) {
        float textWidth = crumb.text.length() * 13.0f * 0.6f;
        float width = textWidth + 16.0f; // padding
        
        crumb.geometry = Rect{ x, m_Geometry.y, width, 24.0f };
        x += width + m_SeparatorSpacing + m_CrumbSpacing;
    }
}

Breadcrumb::CrumbInfo* Breadcrumb::GetCrumbAtPosition(const Point& pos) {
    for (auto& crumb : m_Crumbs) {
        if (pos.x >= crumb.geometry.x && pos.x <= crumb.geometry.x + crumb.geometry.width &&
            pos.y >= crumb.geometry.y && pos.y <= crumb.geometry.y + crumb.geometry.height) {
            return &crumb;
        }
    }
    return nullptr;
}

} // namespace HouseEngine::UI
