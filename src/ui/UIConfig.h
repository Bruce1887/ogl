#pragma once
#include <algorithm>

// --- UI Configuration with Responsive Scaling ---
namespace UIConfig {
    // Reference resolution (design resolution)
    constexpr float REF_WIDTH = 1600.0f;
    constexpr float REF_HEIGHT = 1000.0f;

    // Get scale factors based on current screen size
    inline float scaleX(float screenWidth) { return screenWidth / REF_WIDTH; }
    inline float scaleY(float screenHeight) { return screenHeight / REF_HEIGHT; }
    // Use minimum of X/Y scale to maintain aspect ratio for UI elements
    inline float scaleUniform(float screenWidth, float screenHeight) {
        return std::min(scaleX(screenWidth), scaleY(screenHeight));
    }

    // --- Base Font Sizes (will be scaled) ---
    constexpr int BASE_FONT_SIZE_SMALL = 24;
    constexpr int BASE_FONT_SIZE_MEDIUM = 36;
    constexpr int BASE_FONT_SIZE_LARGE = 48;
    constexpr int BASE_FONT_SIZE_TITLE = 72;
    constexpr int BASE_MENU_FONT_SIZE = BASE_FONT_SIZE_LARGE;

    // Scaled font size helpers
    inline int fontSizeSmall(float sw, float sh) { return std::max(16, (int)(BASE_FONT_SIZE_SMALL * scaleUniform(sw, sh))); }
    inline int fontSizeMedium(float sw, float sh) { return std::max(24, (int)(BASE_FONT_SIZE_MEDIUM * scaleUniform(sw, sh))); }
    inline int fontSizeLarge(float sw, float sh) { return std::max(32, (int)(BASE_FONT_SIZE_LARGE * scaleUniform(sw, sh))); }
    inline int fontSizeTitle(float sw, float sh) { return std::max(48, (int)(BASE_FONT_SIZE_TITLE * scaleUniform(sw, sh))); }

    // --- Base Button Dimensions (at reference resolution) ---
    constexpr float BASE_BUTTON_WIDTH = 250.0f;
    constexpr float BASE_BUTTON_HEIGHT = 60.0f;
    constexpr float BASE_BUTTON_SPACING = 20.0f;

    // Scaled button helpers
    inline float buttonWidth(float sw, float sh) { return BASE_BUTTON_WIDTH * scaleUniform(sw, sh); }
    inline float buttonHeight(float sw, float sh) { return BASE_BUTTON_HEIGHT * scaleUniform(sw, sh); }
    inline float buttonSpacing(float sw, float sh) { return BASE_BUTTON_SPACING * scaleUniform(sw, sh); }

    // --- Base Panel Dimensions (at reference resolution) ---
    constexpr float BASE_PANEL_WIDTH_SMALL = 400.0f;
    constexpr float BASE_PANEL_WIDTH_MEDIUM = 500.0f;
    constexpr float BASE_PANEL_WIDTH_LARGE = 600.0f;
    constexpr float BASE_PANEL_HEIGHT_SMALL = 300.0f;
    constexpr float BASE_PANEL_HEIGHT_MEDIUM = 400.0f;
    constexpr float BASE_PANEL_HEIGHT_LARGE = 500.0f;

    // Scaled panel helpers
    inline float panelWidthSmall(float sw, float sh) { return BASE_PANEL_WIDTH_SMALL * scaleUniform(sw, sh); }
    inline float panelWidthMedium(float sw, float sh) { return BASE_PANEL_WIDTH_MEDIUM * scaleUniform(sw, sh); }
    inline float panelWidthLarge(float sw, float sh) { return BASE_PANEL_WIDTH_LARGE * scaleUniform(sw, sh); }
    inline float panelHeightSmall(float sw, float sh) { return BASE_PANEL_HEIGHT_SMALL * scaleUniform(sw, sh); }
    inline float panelHeightMedium(float sw, float sh) { return BASE_PANEL_HEIGHT_MEDIUM * scaleUniform(sw, sh); }
    inline float panelHeightLarge(float sw, float sh) { return BASE_PANEL_HEIGHT_LARGE * scaleUniform(sw, sh); }

    // --- Base HUD Settings (at reference resolution) ---
    constexpr float BASE_HUD_MARGIN = 50.0f;
    constexpr float BASE_HUD_BAR_WIDTH = 200.0f;
    constexpr float BASE_HUD_BAR_HEIGHT = 20.0f;
    constexpr float BASE_HUD_BAR_SPACING = 10.0f;

    // Scaled HUD helpers
    inline float hudMargin(float sw, float sh) { return BASE_HUD_MARGIN * scaleUniform(sw, sh); }
    inline float hudBarWidth(float sw, float sh) { return BASE_HUD_BAR_WIDTH * scaleUniform(sw, sh); }
    inline float hudBarHeight(float sw, float sh) { return BASE_HUD_BAR_HEIGHT * scaleUniform(sw, sh); }
    inline float hudBarSpacing(float sw, float sh) { return BASE_HUD_BAR_SPACING * scaleUniform(sw, sh); }

    // --- Base Input/Button Sizes for Death Screen (at reference resolution) ---
    constexpr float BASE_INPUT_BOX_WIDTH = 350.0f;
    constexpr float BASE_INPUT_BOX_HEIGHT = 50.0f;
    constexpr float BASE_SUBMIT_BUTTON_WIDTH = 200.0f;
    constexpr float BASE_SUBMIT_BUTTON_HEIGHT = 50.0f;

    // Scaled input/submit helpers
    inline float inputBoxWidth(float sw, float sh) { return BASE_INPUT_BOX_WIDTH * scaleUniform(sw, sh); }
    inline float inputBoxHeight(float sw, float sh) { return BASE_INPUT_BOX_HEIGHT * scaleUniform(sw, sh); }
    inline float submitButtonWidth(float sw, float sh) { return BASE_SUBMIT_BUTTON_WIDTH * scaleUniform(sw, sh); }
    inline float submitButtonHeight(float sw, float sh) { return BASE_SUBMIT_BUTTON_HEIGHT * scaleUniform(sw, sh); }

    // --- Screen Position Ratios (relative to screen size) ---
    // These are multiplied by screen width/height to get actual positions
    constexpr float TITLE_Y_RATIO = 0.12f;       // Title vertical position
    constexpr float MENU_START_Y_RATIO = 0.32f;  // Where menu buttons start
    constexpr float INPUT_BOX_Y_RATIO = 0.48f;   // Input box vertical position
    constexpr float SUBMIT_Y_RATIO = 0.58f;      // Submit button vertical position
    constexpr float CONTINUE_Y_RATIO = 0.68f;    // Continue button vertical position

    // --- Colors (common UI colors) ---
    constexpr float OVERLAY_ALPHA = 0.6f;
    
    // Helper functions for centering
    inline float centerX(float elementWidth, float screenWidth) {
        return (screenWidth - elementWidth) * 0.5f;
    }
    
    inline float centerY(float elementHeight, float screenHeight) {
        return (screenHeight - elementHeight) * 0.5f;
    }
}

// Legacy macro for backward compatibility (uses reference size)
#define MENU_FONT_SIZE UIConfig::BASE_MENU_FONT_SIZE