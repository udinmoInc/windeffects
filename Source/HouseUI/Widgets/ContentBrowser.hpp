#pragma once

#include "../Core/Widget.hpp"
#include "../Core/Style.hpp"
#include "../Core/Icon.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace HouseEngine::UI {

// Content item structure
struct ContentItem {
    std::string id;
    std::string name;
    std::string type; // "folder", "mesh", "material", "texture", etc.
    std::string iconName;
    bool isFolder = false;
    bool isFavorite = false;
    void* userData = nullptr;
};

// View mode for content browser
enum class ContentViewMode {
    Grid,
    List
};

// Content browser widget with grid/list views
class ContentBrowser : public Widget {
public:
    using OnItemDoubleClicked = std::function<void(const ContentItem&)>;
    using OnItemSelected = std::function<void(const ContentItem&)>;
    using OnItemRightClicked = std::function<void(const ContentItem&, const Point&)>;

    ContentBrowser();
    virtual ~ContentBrowser() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void OnMouseDown(const MouseEvent& event) override;
    void OnMouseMove(const MouseEvent& event) override;
    void OnMouseWheel(const MouseEvent& event) override;
    void OnKeyDown(const KeyEvent& event) override;

    // Content management
    void AddItem(const ContentItem& item);
    void RemoveItem(const std::string& id);
    void Clear();
    
    // Selection
    void SetSelectedId(const std::string& id);
    std::string GetSelectedId() const { return m_SelectedId; }
    void ClearSelection() { m_SelectedId.clear(); }
    
    // View mode
    void SetViewMode(ContentViewMode mode) { m_ViewMode = mode; }
    ContentViewMode GetViewMode() const { return m_ViewMode; }
    
    // Callbacks
    void SetOnItemDoubleClicked(OnItemDoubleClicked callback) { m_OnItemDoubleClicked = callback; }
    void SetOnItemSelected(OnItemSelected callback) { m_OnItemSelected = callback; }
    void SetOnItemRightClicked(OnItemRightClicked callback) { m_OnItemRightClicked = callback; }

    // Styling
    void SetGridItemSize(float size) { m_GridItemSize = size; }
    void SetListRowHeight(float height) { m_ListRowHeight = height; }

private:
    struct RenderItem {
        ContentItem item;
        Rect geometry;
    };

    void BuildRenderList();
    void CalculateGridLayout();
    void CalculateListLayout();
    RenderItem* GetItemAtPosition(const Point& pos);

    std::vector<ContentItem> m_Items;
    std::vector<RenderItem> m_RenderList;
    std::string m_SelectedId;
    std::string m_HoveredId;

    ContentViewMode m_ViewMode = ContentViewMode::Grid;
    
    float m_ScrollOffset = 0.0f;
    float m_GridItemSize = 96.0f;
    float m_GridSpacing = 8.0f;
    float m_ListRowHeight = 32.0f;
    float m_ThumbnailSize = 64.0f;

    OnItemDoubleClicked m_OnItemDoubleClicked;
    OnItemSelected m_OnItemSelected;
    OnItemRightClicked m_OnItemRightClicked;

    WidgetStyle m_Style;
};

// Breadcrumb widget for navigation path
class Breadcrumb : public Widget {
public:
    Breadcrumb();
    virtual ~Breadcrumb() = default;

    Size Measure(const Size& availableSize) override;
    void Arrange(const Rect& allottedRect) override;
    void Paint(PaintContext& context) override;

    void OnMouseDown(const MouseEvent& event) override;
    void OnMouseMove(const MouseEvent& event) override;

    void SetPath(const std::vector<std::string>& path);
    void AddCrumb(const std::string& crumb);
    void Clear();

    using OnCrumbClicked = std::function<void(size_t index)>;
    void SetOnCrumbClicked(OnCrumbClicked callback) { m_OnCrumbClicked = callback; }

private:
    struct CrumbInfo {
        std::string text;
        Rect geometry;
        bool hovered = false;
    };

    void CalculateLayout();
    CrumbInfo* GetCrumbAtPosition(const Point& pos);

    std::vector<CrumbInfo> m_Crumbs;
    float m_SeparatorSpacing = 8.0f;
    float m_CrumbSpacing = 4.0f;
    int m_HoveredCrumb = -1;

    OnCrumbClicked m_OnCrumbClicked;
};

} // namespace HouseEngine::UI
