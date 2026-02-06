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

# wxWidgets Best Practices Guide

## Core Programming Practices

**Feature 1: Event Handling**
- **DO:** Use `Bind()` with lambdas or class methods.
- **FORBIDDEN:** Use `DECLARE_EVENT_TABLE` or `Connect()`.
- **Why?** Type safety, flexibility, and cleaner code.

**Feature 2: Object Deletion**
- **DO:** Use `window->Destroy()`.
- **FORBIDDEN:** Use `delete window`.
- **Why?** Destroy prevents crashes by waiting for the event queue to empty.

**Feature 3: Smart Pointers**
- **DO:** Use `std::unique_ptr` for non-window data.
- **FORBIDDEN:** Use `std::shared_ptr` for UI controls.
- **Why?** wxWidgets handles UI parent-child cleanup; shared pointers fight the internal logic.

**Feature 4: String Handling**
- **DO:** Use standard literals `"text"`.
- **FORBIDDEN:** Use `wxT("text")` or `L"text"`.
- **Why?** Modern wxWidgets is Unicode-only; macros are redundant.

**Feature 5: App Startup**
- **DO:** Use `wxIMPLEMENT_APP(MyApp)`.
- **FORBIDDEN:** Use `main()` or `WinMain()`.
- **Why?** The macro handles cross-platform initialization and cleanup for you.

## Layout and UI Design

**Feature 6: Sizing**
- **DO:** Use `wxSizer` for everything.
- **FORBIDDEN:** Hardcode `wxPoint` or `wxSize` pixels.
- **Why?** Hardcoded pixels break on different screen resolutions/DPIs.

**Feature 7: Sizer Syntax**
- **DO:** Use `wxSizerFlags`.
- **FORBIDDEN:** Use bitwise OR flags (e.g., `1, wxALL | wxEXPAND, 5`).
- **Why?** Flags are much easier to read and less prone to errors.

**Feature 8: High DPI**
- **DO:** Use `wxBitmapBundle`.
- **FORBIDDEN:** Use `wxBitmap` or `wxIcon` directly.
- **Why?** Bundles store multiple sizes to keep icons crisp on 4K/Retina displays.

**Feature 9: Spacing**
- **DO:** Use `sizer->AddSpacer(n)`.
- **FORBIDDEN:** Use empty `wxStaticText` for padding.
- **Why?** Spacers are lightweight and designed specifically for layout gaps.

**Feature 10: Theming**
- **DO:** Support System Dark Mode.
- **FORBIDDEN:** Hardcode `*wxWHITE` or `*wxBLACK` backgrounds.
- **Why?** Users expect apps to follow the system theme (Windows 11 / macOS / GTK).

## Threading and Performance

**Feature 11: UI Updates**
- **DO:** Use `CallAfter()` to update UI from threads.
- **FORBIDDEN:** Access UI elements directly from a background thread.
- **Why?** GUI operations are not thread-safe and will cause random crashes.

**Feature 12: Heavy Tasks**
- **DO:** Use `wxThread` or `wxTaskBarIcon`.
- **FORBIDDEN:** Run long loops in the main event thread.
- **Why?** Long loops "freeze" the window, making it non-responsive (Not Responding).

**Feature 13: Paint Events**
- **DO:** Use `wxAutoBufferedPaintDC`.
- **FORBIDDEN:** Use `wxPaintDC` without double-buffering.
- **Why?** Prevents flickering when resizing or redrawing complex custom controls.

## Containers and Data Types

**Feature 1: Containers**
- **DO:** Use `std::vector` or `std::list`.
- **FORBIDDEN:** Use `wxList` or `wxArrayInt`.
- **Why?** Since 3.0, wx containers are mostly wrappers. Standard C++ containers are faster and work with modern algorithms.

**Feature 2: String Conversion**
- **DO:** Use `.ToStdString()` or `wxString::FromUTF8()`.
- **FORBIDDEN:** Use `(const char*)mystring` casts.
- **Why?** Casting is unsafe and fails if the string contains multi-byte characters or if the encoding doesn't match.

**Feature 3: File Paths**
- **DO:** Use `wxFileName`.
- **FORBIDDEN:** Use raw string paths (e.g., `C:\\temp\\`).
- **Why?** wxFileName handles cross-platform separator differences (slash vs backslash) automatically.

**Feature 4: Numbers**
- **DO:** Use `wxString::Format("%d", val)`.
- **FORBIDDEN:** Use `sprintf` or `itoa`.
- **Why?** wxString::Format is type-safe and handles Unicode characters in the format string correctly.

## UI Components and Dialogs

**Feature 5: Dialogs**
- **DO:** Use `wxMessageDialog` with `ShowModal()`.
- **FORBIDDEN:** Create custom frames for simple "OK/Cancel" alerts.
- **Why?** System dialogs look native and handle screen readers/accessibility better than custom ones.

**Feature 6: Input**
- **DO:** Use `wxTextValidator`.
- **FORBIDDEN:** Manually filter key events in `OnChar`.
- **Why?** Validators are cleaner and can automatically filter for "Numeric only" or "Alpha only" without complex logic.

**Feature 7: IDs**
- **DO:** Use `wxID_ANY`.
- **FORBIDDEN:** Hardcode magic numbers like `10001`.
- **Why?** Using wxID_ANY lets the library generate unique IDs, preventing accidental ID collisions in large apps.

**Feature 8: Standard IDs**
- **DO:** Use `wxID_OK`, `wxID_CANCEL`, `wxID_EXIT`.
- **FORBIDDEN:** Define your own `ID_MY_EXIT_BTN`.
- **Why?** Standard IDs automatically hook into platform-specific behaviors (like the "Escape" key closing a dialog).

## Build and Performance Optimization

**Feature 9: Precompiled Headers**
- **DO:** Use `wx/wxprec.h`.
- **FORBIDDEN:** Include every individual header in every file.
- **Why?** wxWidgets is massive; using precompiled headers can cut your build time by 50-80%.

**Feature 10: Asset Loading**
- **DO:** Use `wxEmbeddedFile` or Resources.
- **FORBIDDEN:** Assume icons are in the same folder as the EXE.
- **Why?** Apps are often installed in "Program Files" where they FORBIDDEN have permission to read local loose files easily.

**Feature 11: Logging**
- **DO:** Use `wxLogMessage()` or `wxLogError()`.
- **FORBIDDEN:** Use `std::cout` or `printf`.
- **Why?** wxLog automatically redirects to a neat dialog box in GUI mode but stays in the console for terminal apps.

## Modern Features (3.3.x)

**Feature 12: Dark Mode (Win)**
- **DO:** Use `wxApp::SetAppearance(wxAppearance::System)`.
- **FORBIDDEN:** Try to manually color every window background.
- **Why?** 3.3.x introduces native opt-in dark mode for Windows. Manual coloring usually misses scrollbars and menus.

**Feature 13: Dark Mode Colors**
- **DO:** Use `wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)`.
- **FORBIDDEN:** Hardcode `*wxWHITE` or `wxColour(255, 255, 255)`.
- **Why?** System colors automatically swap when the user toggles Dark/Light mode.

**Feature 14: App Identity**
- **DO:** Use `SetAppDisplayName()` and `SetVendorName()`.
- **FORBIDDEN:** Hardcode "Untitled" or ignore these metadata fields.
- **Why?** This information is used by the OS for task manager grouping and config file locations.

**Feature 15: Bitmaps**
- **DO:** Use `wxBitmapBundle` with SVG support.
- **FORBIDDEN:** Use `.ico` or `.bmp` files for icons.
- **Why?** SVG bundles scale perfectly from 100% to 400% DPI without blurring.

## UI Best Practices

**Feature 16: Panel Usage**
- **DO:** Always put a `wxPanel` inside a `wxFrame`.
- **FORBIDDEN:** Put buttons and text directly on the `wxFrame`.
- **Why?** Frames FORBIDDEN handle tab-traversal (keyboard navigation) or background colors correctly on all platforms.

**Feature 17: Enums/Flags**
- **DO:** Use symbolic flags (e.g., `wxEXEC_ASYNC`).
- **FORBIDDEN:** Use `true` or `false` for mystery boolean args.
- **Why?** Functions like `wxExecute(true)` are unreadable. `wxExecute(wxEXEC_ASYNC)` is self-documenting.

**Feature 18: Virtual Methods**
- **DO:** Use the `override` keyword.
- **FORBIDDEN:** Omit `override` for `OnPaint` or `OnSize`.
- **Why?** override prevents bugs where you think you're overriding a function but actually have a slight typo in the signature.

**Feature 19: Event Propagation**
- **DO:** Use `event.Skip()` to let parents see the event.
- **FORBIDDEN:** Forget `event.Skip()` in a `wxEVT_PAINT` handler.
- **Why?** If you FORBIDDEN `Skip()`, the default system behavior (like highlighting a button) might be blocked.

## Build Tools and Project Structure

**Feature 20: Build Tools**
- **DO:** Use CMake.
- **FORBIDDEN:** Manually maintain `.vcxproj` or Makefiles.
- **Why?** wxWidgets 3.3.x has vastly improved CMake support, making it the fastest way to link the library.

**Feature 21: PCH**
- **DO:** Use Precompiled Headers (`wx/wxprec.h`).
- **FORBIDDEN:** Include `<wx/wx.h>` in every single file.
- **Why?** Using wxprec.h can reduce compilation time by over 60% on large projects.

**Feature 22: Resources**
- **DO:** Use XRC (XML Resources) for UI.
- **FORBIDDEN:** Code every single wxButton placement in C++.
- **Why?** XRC separates your logic from your layout, allowing you to tweak the UI without recompiling.

## Key Principles

### Visualizing the Lifecycle

One of the biggest "FORBIDDENs" is trying to manage the application lifecycle manually. wxWidgets uses a specific startup and shutdown sequence.

### The "Golden Rule" for 3.3.x

If you find yourself writing a Macro, stop and check if there is a Template alternative. Modern wxWidgets has replaced almost all the old macro-based logic with template-based logic that is easier for the compiler to optimize and easier for you to debug.

### Visualizing the UI Hierarchy

In wxWidgets, the relationship between windows is a tree. Understanding this helps you avoid manual memory management.

### Pro-Tip: The "Parent" Rule

When you create a control (like a `wxButton`), you pass a `this` pointer as the parent:
```cpp
new wxButton(this, wxID_ANY, "OK");
```

**The "Do":** Trust the parent. When you `Destroy()` the parent frame, wxWidgets automatically iterates through the children and deletes them properly. You FORBIDDEN need to track them yourself!

### Critical Lifecycle Diagram

If you find yourself wondering "where do I put my cleanup code?" or "why is my frame not showing?", refer to this order of operations.

### The "Golden Rule" for 2026

**Think "Standard C++" first.** In the old days (version 2.4 - 2.8), wxWidgets had to reinvent the wheel because C++ didn't have a standard library for strings, threads, or containers.

- **Today:** If you need a list, use `std::vector`.
- **Today:** If you need a thread, use `std::thread` (and `CallAfter` to talk to the UI).
- **Today:** Use `nullptr` instead of `NULL` or `0`.

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
