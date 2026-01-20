# RME Project Guidelines - C++20 wxWidgets Map Editor

## 🎯 PROJECT VISION

**Modernization Goals:**
1. **C++20** - Modern language features, concepts, ranges, format
2. **wxWidgets Best Practices** - Responsive UI with wxWrapSizer
3. **SOLID Principles** - Single Responsibility, Dependency Inversion
4. **OpenGL Modernization** - Incremental upgrade from 1.x to 4.5
5. **Codebase Decoupling** - Split monolithic files, organize in folders

---

## 🚫 CRITICAL RULES

### 1. APPLICATION.CPP IS OFF-LIMITS
- ONLY app initialization and main loop coordination
- Business logic → proper module/controller

### 2. SEARCH BEFORE CODING
```
☐ Search codebase for similar functionality
☐ Check common/ and utilities
☐ Reuse, don't duplicate
```

### 3. SINGLE RESPONSIBILITY
- File > 500 lines → **SPLIT IT**
- Function > 50 lines → **SPLIT IT**
- Class doing multiple things → **SPLIT IT**

---

## 🏗️ C++20 MODERNIZATION

### Use Modern Features
```cpp
// ✅ std::format instead of sprintf/wxString::Format
std::format("Position: {}, {}", x, y)

// ✅ Concepts for templates
template<typename T> requires std::integral<T>
void snap_to_grid(T& value, T grid_size);

// ✅ std::span instead of pointer+size
void process_tiles(std::span<Tile*> tiles);

// ✅ Designated initializers
Position pos{.x = 10, .y = 20, .z = 7};

// ✅ Range-based algorithms
std::ranges::for_each(items, [](auto& item) { ... });

// ✅ auto + structured bindings
auto [x, y, z] = position.get_coords();
```

### Memory Management
```cpp
// ✅ Smart pointers for ownership
std::unique_ptr<Tile> tile = std::make_unique<Tile>(...);

// ✅ Raw pointers for observation only
Tile* observe(const Tile& source);

// ❌ BANNED - raw new/delete
Tile* t = new Tile();  // NO!
delete t;              // NO!
```

---

## 🖼️ wxWidgets Best Practices

### Layout - wxWrapSizer is MANDATORY for Tilesets
```cpp
// ✅ CORRECT - Responsive tileset grid
wxWrapSizer* sizer = new wxWrapSizer(wxHORIZONTAL);
for (auto& brush : brushes) {
    sizer->Add(CreateBrushPanel(brush), 0, wxALL, 2);
}

// ❌ WRONG - Fixed grid that doesn't adapt
wxGridSizer* grid = new wxGridSizer(4, 4, 5, 5);  // NO for tilesets!
```

### Event Handling
```cpp
// ✅ Bind() instead of event tables
Bind(wxEVT_BUTTON, &MyClass::OnButtonClick, this, ID_BUTTON);
Bind(wxEVT_SIZE, [this](wxSizeEvent& e) { Layout(); e.Skip(); });

// ❌ Old-style event tables (avoid in new code)
BEGIN_EVENT_TABLE(...)  // Legacy only
```

### Virtual List Controls for Large Data
```cpp
// ✅ wxListCtrl in virtual mode for 1000+ items
class ItemListCtrl : public wxListCtrl {
    wxString OnGetItemText(long item, long column) const override;
};
SetItemCount(items.size());

// ❌ Adding thousands of items directly
for (auto& item : thousands) list->InsertItem(...);  // SLOW!
```

### UI Thread Safety
```cpp
// ✅ CallAfter for cross-thread UI updates
wxGetApp().CallAfter([this]() {
    UpdateStatusBar();
});

// ❌ Direct UI updates from worker threads
statusBar->SetLabel(...);  // CRASH!
```

---

## 🎮 OpenGL Modernization (1.x → 4.5)

### Current State (Legacy)
```cpp
// Existing code uses immediate mode (OpenGL 1.x)
glBegin(GL_QUADS);
glVertex2f(...);
glEnd();
```

### Target State (Modern)
```cpp
// ✅ VAO/VBO approach (OpenGL 3.3+)
GLuint vao, vbo;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);
glGenBuffers(1, &vbo);
// ...batch geometry upload

// ✅ RAII wrappers
class VertexBuffer {
    GLuint id;
public:
    VertexBuffer() { glGenBuffers(1, &id); }
    ~VertexBuffer() { glDeleteBuffers(1, &id); }
};
```

### Migration Strategy
1. **Phase 1:** Add RAII wrappers for GL objects
2. **Phase 2:** Create shader abstraction layer
3. **Phase 3:** Migrate rendering to batched VBOs
4. **Phase 4:** Add modern effects (lighting, etc.)

---

## 📁 FILE ORGANIZATION

### Target Structure
```
source/
├── core/           # Application, Editor, Map
├── brushes/        # All brush implementations
├── ui/
│   ├── windows/    # Dialog windows
│   ├── palettes/   # Palette panels
│   └── controls/   # Custom controls
├── io/             # File I/O (OTBM, etc.)
├── rendering/      # OpenGL, drawing
├── data/           # Item, Creature, Tile
├── network/        # Live editing
└── utils/          # Common utilities
```

### File Naming
| Pattern | Location |
|---------|----------|
| `*_window.cpp` | `ui/windows/` |
| `*_brush.cpp` | `brushes/` |
| `palette_*.cpp` | `ui/palettes/` |
| `iomap_*.cpp` | `io/` |
| `live_*.cpp` | `network/` |

---

## ✅ PRE-COMMIT CHECKLIST

```
☐ No code added to application.cpp
☐ No duplicate code created
☐ Functions < 50 lines
☐ Files < 500 lines (or has refactoring plan)
☐ Smart pointers for new allocations
☐ wxWrapSizer for tileset layouts
☐ C++20 features where applicable
☐ RAII for any new OpenGL resources
```

---

## 🔄 INCREMENTAL IMPROVEMENTS

When touching a file, apply these improvements:
1. Replace `NULL` with `nullptr`
2. Use `auto` where type is obvious
3. Use range-based for loops
4. Add `const` to non-mutating methods
5. Replace C-style casts with C++ casts
6. Use `override` on virtual functions
7. Use `= default` and `= delete`

---

## 📌 THE MANTRA

**SEARCH → REUSE → ORGANIZE → MODERNIZE → IMPLEMENT**
