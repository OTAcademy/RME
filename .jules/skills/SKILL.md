---
name: RME Modern UI System
description: Comprehensive standards for building premium, high-performance UI components in Remere's Map Editor using NanoVG and wxWidgets.
---

# RME Modern UI Design Standards

The "Advanced Replace Tool" serves as the golden standard for the project's look and feel. This skill documents the styling, architecture, and implementation patterns required to maintain this level of quality across the entire editor.

## 1. Design Philosophy
- **Rich Aesthetics**: Use vibrant, curated dark-mode palettes. Avoid flat colors; use subtle gradients and shadows.
- **Visual Excellence**: Prioritize high-quality rendering (rounded corners, smooth anti-aliased text, wxGraphicsContext for premium effects).
- **Dynamic Interaction**: Every interactive element must provide visual feedback (hover highlights, active states).
- **Modern Typography**: Use `sans` font with appropriate weight and wrapping for readability.
- **Responsive Layout**: Use flex-based column layouts with proper proportions (e.g., 3:4:3:2 ratio).

## 2. Color Palette & Theming
Always use the `Theme` class for unified coloring. Never hardcode colors unless defining a new role.

### Theme Roles
| Role | Color (RGBA) | Usage |
| :--- | :--- | :--- |
| `Surface` | `rgba(45, 45, 48, 255)` | Main panel backgrounds. |
| `Background` | `rgba(30, 30, 30, 255)` | Darker areas like grids and lists. |
| `Header` | `rgba(25, 25, 25, 255)` | Section headers. |
| `Accent` | `rgba(0, 120, 215, 255)` | Primary actions, selection borders, active states. |
| `AccentHover` | `rgba(0, 150, 255, 255)` | Lighter accent for hover states. |
| `Text` | `rgba(230, 230, 230, 255)` | Primary labels. |
| `TextSubtle` | `rgba(150, 150, 150, 255)` | Secondary labels, descriptions. |
| `Border` | `rgba(60, 60, 60, 255)` | Separators and inactive outlines. |
| `Selected` | `rgba(0, 120, 215, 60)` | Selection fill with alpha. |
| `Error` | `rgba(200, 50, 50, 255)` | Delete buttons, warnings. |

### Card-Specific Colors
```cpp
wxColour cardBgStart = wxColour(45, 45, 52); // Slightly bluish dark
wxColour cardBgEnd = wxColour(40, 40, 48);
wxColour borderColor = wxColour(60, 60, 70);
wxColour headerBg = wxColour(35, 35, 40);
wxColour titleColor = wxColour(220, 220, 230);
```

### NanoVG Card Gradients
Interactive cards should use a linear gradient for a premium feel:
- **Top**: `rgba(60, 60, 65, 255)`
- **Bottom**: `rgba(50, 50, 55, 255)`

## 3. Layout Standards
All measurements must be wrapped in `FromDIP()` for DPI awareness.

### Global Constants
```cpp
static const int CARD_PADDING = 20;      // Internal padding within cards
static const int CARD_MARGIN_X = 10;     // Horizontal spacing between cards
static const int CARD_MARGIN_Y = 10;     // Vertical spacing between cards
static const int HEADER_HEIGHT = 40;     // Fixed header height
static const int FOOTER_HEIGHT = 40;     // Fixed footer height
static const int ITEM_SIZE = 56;         // Item icon size (golden standard)
static const int ITEM_H = 110;           // Item card height (for text + icon)
static const int ITEM_SPACING = 10;      // Spacing between items in grids
static const int ARROW_WIDTH = 60;       // Width of arrow indicators
static const int SECTION_GAP = 20;       // Gap between major sections
```

### Rounded Corners
- **Standard**: `4.0f` (buttons, small cards)
- **Large Cards**: `6.0` (CardPanel outer radius)

### Column Flex Ratios
For multi-column layouts (like Replace Tool):
```cpp
// Example: 4-column layout with flex weights
mainRowSizer->Add(col1Card, 3, wxEXPAND); // Item Library: 3 units
mainRowSizer->Add(col2Card, 4, wxEXPAND); // Rule Builder: 4 units (largest)
mainRowSizer->Add(col3Card, 3, wxEXPAND); // Smart Suggestions: 3 units
mainRowSizer->Add(col4Card, 2, wxEXPAND); // Saved Rules: 2 units (smallest)
```

## 4. wxWidgets Patterns

### CardPanel Component
The `CardPanel` is the foundational container for all major sections. It provides:
- Rounded corners with gradient backgrounds
- Optional header with centered title
- Optional footer for action buttons
- Shadow effect using wxGraphicsContext

**Reference**: [card_panel.cpp](source/ui/replace_tool/card_panel.cpp)

#### Usage Example
```cpp
CardPanel* card = new CardPanel(parent, wxID_ANY);
card->SetTitle("SECTION TITLE");
card->SetShowFooter(true);

// Add content to card
card->GetContentSizer()->Add(myControl, 1, wxEXPAND | wxALL, padding);

// Add footer buttons
wxBoxSizer* footerSizer = new wxBoxSizer(wxHORIZONTAL);
footerSizer->AddStretchSpacer(1);
footerSizer->Add(myButton, 0, wxALL, padding/2);
footerSizer->AddStretchSpacer(1);
card->GetFooterSizer()->Add(footerSizer, 1, wxEXPAND);
```

#### CardPanel Rendering Details
```cpp
// Shadow effect (offset by 2px)
gc->SetBrush(wxBrush(wxColour(0, 0, 0, 40)));
gc->DrawRoundedRectangle(margin + 2, margin + 2, w - 2*margin, h - 2*margin, r);

// Gradient background
wxGraphicsBrush brush = gc->CreateLinearGradientBrush(
    margin, margin, margin, h - margin, 
    cardBgStart, cardBgEnd
);
gc->SetBrush(brush);
gc->FillPath(path);
```

### Button Styling
```cpp
// Primary action button (accent color)
m_executeBtn->SetBackgroundColour(Theme::Get(Theme::Role::Accent));
m_executeBtn->SetForegroundColour(*wxWHITE);
m_executeBtn->SetDefault();

// Standard buttons use default theme colors
```

### Responsive Sizing
```cpp
// Use FromDIP for all fixed sizes
wxButton* btn = new wxButton(parent, wxID_ANY, "Label", 
                             wxDefaultPosition, FromDIP(wxSize(60, -1)));
```

## 5. OpenGL Context Management & NanoVG Base Class

### NanoVGCanvas Base Class
All custom NanoVG controls **must** inherit from `NanoVGCanvas`, which handles:
- OpenGL context creation and management
- NanoVG initialization with antialiasing
- Texture caching for performance
- Virtual scrolling support
- DPI-aware rendering

**Reference**: [nanovg_canvas.h](source/util/nanovg_canvas.h) | [nanovg_canvas.cpp](source/util/nanovg_canvas.cpp)

#### Creating a Custom NanoVG Control
```cpp
class MyCustomPanel : public NanoVGCanvas {
public:
    MyCustomPanel(wxWindow* parent) 
        : NanoVGCanvas(parent, wxID_ANY, wxVSCROLL | wxWANTS_CHARS) {
        // wxVSCROLL enables vertical scrolling
        // wxWANTS_CHARS allows keyboard input
    }

protected:
    void OnNanoVGPaint(NVGcontext* vg, int width, int height) override {
        // Your custom drawing code here
        // Coordinate system is already translated by -scrollPos
        
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 10, 10, 100, 50, 4.0f);
        nvgFillColor(vg, nvgRGBA(80, 80, 80, 255));
        nvgFill(vg);
    }
    
    wxSize DoGetBestClientSize() const override {
        return FromDIP(wxSize(200, 200));
    }
};
```

### OpenGL Context Management (Critical!)
**Always** use `ScopedGLContext` when creating textures outside of `OnNanoVGPaint()`:

```cpp
// CORRECT: Using ScopedGLContext for texture creation
void MyPanel::LoadItemTexture(uint16_t itemId) {
    ScopedGLContext ctx(this);  // Makes GL context current
    
    // Now safe to call NanoVG texture functions
    int tex = GetOrCreateImage(itemId, pixelData, width, height);
    
    // Context automatically restored when ctx goes out of scope
}

// WRONG: Missing context activation
void MyPanel::LoadItemTexture(uint16_t itemId) {
    // This will crash or corrupt textures!
    int tex = GetOrCreateImage(itemId, pixelData, width, height);
}
```

### Texture Caching Pattern
The `NanoVGCanvas` base class provides automatic texture caching:

```cpp
// Get or create a texture (automatically cached)
int tex = GetOrCreateImage(itemId, rgbaData, width, height);

// Use the texture
if (tex > 0) {
    NVGpaint imgPaint = nvgImagePattern(vg, x, y, w, h, 0, tex, 1.0f);
    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFillPaint(vg, imgPaint);
    nvgFill(vg);
}

// Delete a specific cached texture
DeleteCachedImage(itemId);

// Clear all cached textures (e.g., when data changes)
ClearImageCache();
```

### NanoVGCanvas Initialization Flow
```cpp
// 1. Constructor creates wxGLCanvas
NanoVGCanvas::NanoVGCanvas(wxWindow* parent, wxWindowID id, long style)
    : wxGLCanvas(parent, id, nullptr, wxDefaultPosition, wxDefaultSize, style) {
    // Binds paint, size, scroll events
}

// 2. First OnPaint() triggers InitGL()
void NanoVGCanvas::InitGL() {
    m_glContext = std::make_unique<wxGLContext>(this);
    SetCurrent(*m_glContext);
    
    gladLoadGL();  // Load OpenGL functions
    
    m_nvg.reset(nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES));
    TextRenderer::LoadFont(m_nvg.get());  // Load default font
}

// 3. Every paint calls SetCurrent() before rendering
void NanoVGCanvas::OnPaint(wxPaintEvent&) {
    SetCurrent(*m_glContext);
    
    nvgBeginFrame(vg, w, h, 1.0f);
    OnNanoVGPaint(vg, w, h);  // Your custom drawing
    nvgEndFrame(vg);
    
    SwapBuffers();
}
```

## 6. NanoVG Implementation Patterns

### Card with Gradient & Shadow
```cpp
// 1. Shadow (if hovered/selected)
NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, 4.0f, 6.0f, 
                                       nvgRGBA(0, 0, 0, 64), nvgRGBA(0, 0, 0, 0));
nvgBeginPath(vg);
nvgRect(vg, x - 5, y - 5, w + 10, h + 10);
nvgRoundedRect(vg, x, y, w, h, 4.0f);
nvgPathWinding(vg, NVG_HOLE);
nvgFillPaint(vg, shadowPaint);
nvgFill(vg);

// 2. Background Gradient
NVGpaint bgPaint = nvgLinearGradient(vg, x, y, x, y + h, 
                                      nvgRGBA(60, 60, 65, 255), 
                                      nvgRGBA(50, 50, 55, 255));
nvgBeginPath(vg);
nvgRoundedRect(vg, x, y, w, h, 4.0f);
nvgFillPaint(vg, bgPaint);
nvgFill(vg);

// 3. Border (if selected)
nvgBeginPath(vg);
nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, 4.0f);
nvgStrokeColor(vg, nvgRGBA(100, 180, 255, 255));
nvgStrokeWidth(vg, 2.0f);
nvgStroke(vg);
```

### Wrapped Text with Masking
Item names must be wrapped to at least 2 lines to avoid truncation.
```cpp
nvgFontSize(vg, 11.0f);
nvgFontFace(vg, "sans");
nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

// Use nvgTextBox for automatic wrapping
float textX = x + 5;
float textY = y + 44;  // Below icon area
float textW = w - 10;
nvgTextBox(vg, textX, textY, textW, item_name, nullptr);
```

### Delete Icon (Corner X)
```cpp
float cx = x + w - 8;  // Top-right corner
float cy = y + 8;
float r = 8.0f;

// Circle background
nvgBeginPath(vg);
nvgCircle(vg, cx, cy, r);
nvgFillColor(vg, nvgRGBA(200, 50, 50, 255));
nvgFill(vg);

// X mark
float crossArr = r * 0.5f;
nvgBeginPath(vg);
nvgMoveTo(vg, cx - crossArr, cy - crossArr);
nvgLineTo(vg, cx + crossArr, cy + crossArr);
nvgMoveTo(vg, cx + crossArr, cy - crossArr);
nvgLineTo(vg, cx - crossArr, cy + crossArr);
nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
nvgStrokeWidth(vg, 1.5f);
nvgStroke(vg);
```

### Hover States
```cpp
// Track hover in OnMotion
void OnMotion(wxMouseEvent& event) {
    HitResult hit = HitTest(event.GetX(), event.GetY());
    if (hit != m_hoverState) {
        m_hoverState = hit;
        Refresh();
    }
}

// Render differently based on hover
if (isHovered) {
    nvgFillColor(vg, nvgRGBA(70, 70, 75, 255));
} else {
    // Normal gradient
}
```

## 7. Architectural Patterns

### Decoupling with Listeners
Always use a `Listener` interface to communicate events from components to parent windows.
```cpp
class MyComponent : public NanoVGCanvas {
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void OnActionTriggered(uint16_t id) = 0;
        virtual void OnSelectionChanged() = 0;
    };
    
    MyComponent(wxWindow* parent, Listener* listener) 
        : NanoVGCanvas(parent), m_listener(listener) {}
        
private:
    Listener* m_listener;
};
```

### DRY: Unified Item Rendering
Never draw items manually. Use the centralized utility to create textures:
```cpp
int tex = NvgUtils::CreateItemTexture(vg, itemId); 
// Reference: util/nvg_utils.h
```

### Hit Testing Pattern
Implement precise hit testing for interactive elements:
```cpp
struct HitResult {
    enum Type {
        None,
        Source,
        Target,
        AddTarget,
        DeleteRule,
        DeleteTarget
    };
    Type type = None;
    int ruleIndex = -1;
    int targetIndex = -1;
};

HitResult HitTest(int x, int y) const {
    // Convert to scrolled coordinates
    int absY = y + GetScrollPosition();
    
    // Test each interactive region
    // Return specific hit result with indices
}
```

### Drag & Drop Implementation
```cpp
class ItemDropTarget : public wxTextDropTarget {
public:
    ItemDropTarget(MyPanel* panel) : m_panel(panel) {}
    
    bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override {
        if (!data.StartsWith("RME_ITEM:")) return false;
        
        unsigned long idVal;
        if (!data.AfterFirst(':').ToULong(&idVal)) return false;
        uint16_t itemId = (uint16_t)idVal;
        
        HitResult hit = m_panel->HitTest(x, y);
        // Handle drop based on hit result
        return true;
    }
    
    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override {
        m_panel->UpdateDragHover(x, y);
        return wxDragMove;
    }
    
    void OnLeave() override {
        m_panel->ClearDragHover();
    }
};

// In component constructor:
SetDropTarget(new ItemDropTarget(this));
```

### Scrollable NanoVG Panels
```cpp
class MyPanel : public NanoVGCanvas {
public:
    MyPanel(wxWindow* parent) 
        : NanoVGCanvas(parent, wxID_ANY, wxVSCROLL | wxWANTS_CHARS) {
        // wxVSCROLL enables vertical scrolling
        // wxWANTS_CHARS allows keyboard input
    }
    
protected:
    void OnNanoVGPaint(NVGcontext* vg, int width, int height) override {
        int scrollPos = GetScrollPosition();
        
        // Draw fixed header (not scrolled)
        nvgSave(vg);
        DrawHeader(vg, width);
        nvgRestore(vg);
        
        // Translate for scrolled content
        nvgTranslate(vg, 0, -scrollPos);
        DrawScrolledContent(vg, width);
    }
};
```

## 8. Common UI Patterns

### Fixed Header with Scrollable Content
```cpp
// Draw header at fixed position
nvgSave(vg);
nvgFontSize(vg, 10.0f);
nvgFillColor(vg, subTextCol);
nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
nvgText(vg, CARD_MARGIN_X, HEADER_HEIGHT / 2.0f, "SECTION LABEL", nullptr);
nvgRestore(vg);

// Translate for scrolled content
nvgTranslate(vg, 0, -scrollPos);
```

### Ghost Slot (Add New Item)
```cpp
// Draw dashed border for empty slot
nvgBeginPath(vg);
nvgRoundedRect(vg, x, y, w, h, 4.0f);
nvgStrokeColor(vg, nvgRGBA(100, 100, 100, 128));
nvgStrokeWidth(vg, 1.0f);
nvgStroke(vg);

// Draw "+" icon
nvgFontSize(vg, 32.0f);
nvgFillColor(vg, nvgRGBA(150, 150, 150, 255));
nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
nvgText(vg, x + w/2, y + h/2, "+", nullptr);
```

### Probability Display
```cpp
if (probability >= 0) {
    std::string probLabel = std::format("Chance: {}%", probability);
    nvgFillColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgFontSize(vg, 11.0f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgText(vg, x + w / 2, y + 84, probLabel.c_str(), nullptr);
}
```

## 9. Golden Standard Reference Files
Refer to these files for the definitive implementation:

### Core Components
- **Theme System**: [theme.h](source/ui/theme.h)
- **Card Container**: [card_panel.cpp](source/ui/replace_tool/card_panel.cpp)
- **Complex UI Drawing**: [rule_builder_panel.cpp](source/ui/replace_tool/rule_builder_panel.cpp)
- **Virtual Grid & Texture Management**: [virtual_item_grid.cpp](source/util/virtual_item_grid.cpp)
- **Component Packaging**: [item_grid_panel.cpp](source/ui/replace_tool/item_grid_panel.cpp)
- **Window Layout**: [replace_tool_window.cpp](source/ui/replace_tool/replace_tool_window.cpp)

### Key Utilities
- **Item Texture Creation**: `util/nvg_utils.h` - `NvgUtils::CreateItemTexture()`
- **DPI Scaling**: `wxWindow::FromDIP()` - Always use for fixed sizes
- **Theme Access**: `Theme::Get(Theme::Role::*)` - Centralized color management

## 10. Implementation Checklist

When creating a new UI component, ensure:

- [ ] All colors use `Theme::Get()` or documented card-specific colors
- [ ] All fixed sizes use `FromDIP()`
- [ ] Layout uses flex-based sizers with appropriate ratios
- [ ] NanoVG cards use gradient backgrounds (60,60,65 → 50,50,55)
- [ ] Rounded corners are 4.0f for small elements, 6.0 for large cards
- [ ] Text uses `nvgTextBox()` for wrapping, not `nvgText()`
- [ ] Item textures use `NvgUtils::CreateItemTexture()`
- [ ] Interactive elements implement hover states
- [ ] Listener pattern used for parent communication
- [ ] Hit testing implemented for clickable regions
- [ ] Drag & drop uses `wxTextDropTarget` with "RME_ITEM:" prefix
- [ ] Scrollable panels use `wxVSCROLL` flag
- [ ] Headers/footers are fixed (not scrolled)
