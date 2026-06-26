#pragma once

#include "Geometry.hpp"
#include <string>
#include <memory>

namespace HouseEngine::UI {

struct Theme {
    // ------------------------------------------------------------------------
    // Colors (Premium M3-inspired Dark Mode)
    // ------------------------------------------------------------------------
    Color PrimaryBackground = { 0.035f, 0.035f, 0.043f, 1.0f }; // #09090B (Deep background)
    Color PanelBackground   = { 0.094f, 0.094f, 0.105f, 0.85f }; // #18181B with glassmorphism transparency
    Color ToolbarBackground = { 0.094f, 0.094f, 0.105f, 0.95f }; // #18181B slightly more opaque
    
    Color HoverOverlay      = { 0.152f, 0.152f, 0.164f, 1.0f }; // #27272A
    Color SelectedAccent    = { 0.545f, 0.360f, 0.964f, 1.0f }; // #8B5CF6 (Vibrant Purple/Violet)
    
    Color TextPrimary       = { 0.980f, 0.980f, 0.980f, 1.0f }; // #FAFAFA
    Color TextSecondary     = { 0.631f, 0.631f, 0.666f, 1.0f }; // #A1A1AA
    
    Color BorderDefault     = { 0.152f, 0.152f, 0.164f, 1.0f }; // #27272A (Subtle border)
    
    // Status Colors
    Color Error             = { 0.937f, 0.266f, 0.266f, 1.0f }; // #EF4444 (Vibrant Red)
    Color Warning           = { 0.964f, 0.584f, 0.117f, 1.0f }; // #F59E0B (Vibrant Amber)
    Color Success           = { 0.133f, 0.772f, 0.368f, 1.0f }; // #22C55E (Vibrant Green)

    // ------------------------------------------------------------------------
    // Typography
    // ------------------------------------------------------------------------
    float TextSizeMenu      = 14.0f;
    float TextSizeToolbar   = 14.0f;
    float TextSizeHeader    = 15.0f;
    float TextSizeBody      = 14.0f;
    float TextSizeSmall     = 12.0f;

    // ------------------------------------------------------------------------
    // Layout & Styling
    // ------------------------------------------------------------------------
    float CornerRadiusSmall = 6.0f;
    float CornerRadiusLarge = 12.0f;
    
    float BorderWidth       = 1.5f;
    
    Margin PaddingWindow    = { 12.0f, 12.0f, 12.0f, 12.0f };
    Margin PaddingPanel     = { 8.0f, 8.0f, 8.0f, 8.0f };
    Margin PaddingButton    = { 16.0f, 8.0f, 16.0f, 8.0f };
    Margin PaddingIconBtn   = { 6.0f, 6.0f, 6.0f, 6.0f };

    // ------------------------------------------------------------------------
    // Global Management
    // ------------------------------------------------------------------------
    static Theme& Get() {
        static Theme instance;
        return instance;
    }
};

} // namespace HouseEngine::UI
