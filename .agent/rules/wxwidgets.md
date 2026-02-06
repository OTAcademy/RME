---
description: wxWidgets UI Best Practices for RME Map Editor
trigger: always_on
---

# wxWidgets Best Practices Guide

**Feature 1: Event Handling**
- **MANDATORY:** Use `Bind()` with lambdas or class methods.
- **FORBIDDEN:** Use `DECLARE_EVENT_TABLE` or `Connect()`.
- **Why?** Type safety, flexibility, and cleaner code.

**Feature 2: Object Deletion**
- **MANDATORY:** Use `window->Destroy()`.
- **FORBIDDEN:** Use `delete window`.
- **Why?** Destroy prevents crashes by waiting for the event queue to empty.

**Feature 3: Smart Pointers**
- **MANDATORY:** Use `std::unique_ptr` for non-window data.
- **FORBIDDEN:** Use `std::shared_ptr` for UI controls.
- **Why?** wxWidgets handles UI parent-child cleanup; shared pointers fight the internal logic.

**Feature 4: String Handling**
- **MANDATORY:** Use standard literals `"text"`.
- **FORBIDDEN:** Use `wxT("text")` or `L"text"`.
- **Why?** Modern wxWidgets is Unicode-only; macros are redundant.

**Feature 5: App Startup**
- **MANDATORY:** Use `wxIMPLEMENT_APP(MyApp)`.
- **FORBIDDEN:** Use `main()` or `WinMain()`.
- **Why?** The macro handles cross-platform initialization and cleanup for you.

## Layout and UI Design

**Feature 6: Sizing**
- **MANDATORY:** Use `wxSizer` for everything.
- **FORBIDDEN:** Hardcode `wxPoint` or `wxSize` pixels.
- **Why?** Hardcoded pixels break on different screen resolutions/DPIs.

**Feature 7: Sizer Syntax**
- **MANDATORY:** Use `wxSizerFlags`.
- **FORBIDDEN:** Use bitwise OR flags (e.g., `1, wxALL | wxEXPAND, 5`).
- **Why?** Flags are much easier to read and less prone to errors.

**Feature 8: High DPI**
- **MANDATORY:** Use `wxBitmapBundle`.
- **FORBIDDEN:** Use `wxBitmap` or `wxIcon` directly.
- **Why?** Bundles store multiple sizes to keep icons crisp on 4K/Retina displays.

**Feature 9: Spacing**
- **MANDATORY:** Use `sizer->AddSpacer(n)`.
- **FORBIDDEN:** Use empty `wxStaticText` for padding.
- **Why?** Spacers are lightweight and designed specifically for layout gaps.

**Feature 10: Theming**
- **MANDATORY:** Support System Dark Mode.
- **FORBIDDEN:** Hardcode `*wxWHITE` or `*wxBLACK` backgrounds.
- **Why?** Users expect apps to follow the system theme (Windows 11 / macOS / GTK).

## Threading and Performance

**Feature 11: UI Updates**
- **MANDATORY:** Use `CallAfter()` to update UI from threads.
- **FORBIDDEN:** Access UI elements directly from a background thread.
- **Why?** GUI operations are not thread-safe and will cause random crashes.

**Feature 12: Heavy Tasks**
- **MANDATORY:** Use `wxThread` or `wxTaskBarIcon`.
- **FORBIDDEN:** Run long loops in the main event thread.
- **Why?** Long loops "freeze" the window, making it non-responsive (Not Responding).

**Feature 13: Paint Events**
- **MANDATORY:** Use `wxAutoBufferedPaintDC`.
- **FORBIDDEN:** Use `wxPaintDC` without double-buffering.
- **Why?** Prevents flickering when resizing or redrawing complex custom controls.

## Containers and Data Types

**Feature 14: Containers**
- **MANDATORY:** Use `std::vector` or `std::list`.
- **FORBIDDEN:** Use `wxList` or `wxArrayInt`.
- **Why?** Since 3.0, wx containers are mostly wrappers. Standard C++ containers are faster and work with modern algorithms.

**Feature 15: String Conversion**
- **MANDATORY:** Use `.ToStdString()` or `wxString::FromUTF8()`.
- **FORBIDDEN:** Use `(const char*)mystring` casts.
- **Why?** Casting is unsafe and fails if the string contains multi-byte characters or if the encoding doesn't match.

**Feature 16: File Paths**
- **MANDATORY:** Use `wxFileName`.
- **FORBIDDEN:** Use raw string paths (e.g., `C:\\temp\\`).
- **Why?** wxFileName handles cross-platform separator differences (slash vs backslash) automatically.

**Feature 17: Numbers**
- **MANDATORY:** Use `wxString::Format("%d", val)`.
- **FORBIDDEN:** Use `sprintf` or `itoa`.
- **Why?** wxString::Format is type-safe and handles Unicode characters in the format string correctly.

## UI Components and Dialogs

**Feature 18: Dialogs**
- **MANDATORY:** Use `wxMessageDialog` with `ShowModal()`.
- **FORBIDDEN:** Create custom frames for simple "OK/Cancel" alerts.
- **Why?** System dialogs look native and handle screen readers/accessibility better than custom ones.

**Feature 19: Input**
- **MANDATORY:** Use `wxTextValidator`.
- **FORBIDDEN:** Manually filter key events in `OnChar`.
- **Why?** Validators are cleaner and can automatically filter for "Numeric only" or "Alpha only" without complex logic.

**Feature 20: IDs**
- **MANDATORY:** Use `wxID_ANY`.
- **FORBIDDEN:** Hardcode magic numbers like `10001`.
- **Why?** Using wxID_ANY lets the library generate unique IDs, preventing accidental ID collisions in large apps.

**Feature 21: Standard IDs**
- **MANDATORY:** Use `wxID_OK`, `wxID_CANCEL`, `wxID_EXIT`.
- **FORBIDDEN:** Define your own `ID_MY_EXIT_BTN`.
- **Why?** Standard IDs automatically hook into platform-specific behaviors (like the "Escape" key closing a dialog).

## Build and Performance Optimization

**Feature 22: Precompiled Headers**
- **MANDATORY:** Use `wx/wxprec.h`.
- **FORBIDDEN:** Include every individual header in every file.
- **Why?** wxWidgets is massive; using precompiled headers can cut your build time by 50-80%.

**Feature 23: Asset Loading**
- **MANDATORY:** Use `wxEmbeddedFile` or Resources.
- **FORBIDDEN:** Assume icons are in the same folder as the EXE.
- **Why?** Apps are often installed in "Program Files" where they don't have permission to read local loose files easily.

**Feature 24: Logging**
- **MANDATORY:** Use `wxLogMessage()` or `wxLogError()`.
- **FORBIDDEN:** Use `std::cout` or `printf`.
- **Why?** wxLog automatically redirects to a neat dialog box in GUI mode but stays in the console for terminal apps.

## Modern Features (3.3.x)

**Feature 25: Dark Mode (Win)**
- **MANDATORY:** Use `wxApp::SetAppearance(wxAppearance::System)`.
- **FORBIDDEN:** Try to manually color every window background.
- **Why?** 3.3.x introduces native opt-in dark mode for Windows. Manual coloring usually misses scrollbars and menus.

**Feature 26: Dark Mode Colors**
- **MANDATORY:** Use `wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)`.
- **FORBIDDEN:** Hardcode `*wxWHITE` or `wxColour(255, 255, 255)`.
- **Why?** System colors automatically swap when the user toggles Dark/Light mode.

**Feature 27: App Identity**
- **MANDATORY:** Use `SetAppDisplayName()` and `SetVendorName()`.
- **FORBIDDEN:** Hardcode "Untitled" or ignore these metadata fields.
- **Why?** This information is used by the OS for task manager grouping and config file locations.

**Feature 28: Bitmaps**
- **MANDATORY:** Use `wxBitmapBundle` with SVG support.
- **FORBIDDEN:** Use `.ico` or `.bmp` files for icons.
- **Why?** SVG bundles scale perfectly from 100% to 400% DPI without blurring.

## UI Best Practices

**Feature 29: Panel Usage**
- **MANDATORY:** Always put a `wxPanel` inside a `wxFrame`.
- **FORBIDDEN:** Put buttons and text directly on the `wxFrame`.
- **Why?** Frames don't handle tab-traversal (keyboard navigation) or background colors correctly on all platforms.

**Feature 30: Enums/Flags**
- **MANDATORY:** Use symbolic flags (e.g., `wxEXEC_ASYNC`).
- **FORBIDDEN:** Use `true` or `false` for mystery boolean args.
- **Why?** Functions like `wxExecute(true)` are unreadable. `wxExecute(wxEXEC_ASYNC)` is self-documenting.

**Feature 31: Virtual Methods**
- **MANDATORY:** Use the `override` keyword.
- **FORBIDDEN:** Omit `override` for `OnPaint` or `OnSize`.
- **Why?** override prevents bugs where you think you're overriding a function but actually have a slight typo in the signature.

**Feature 32: Event Propagation**
- **MANDATORY:** Use `event.Skip()` to let parents see the event.
- **FORBIDDEN:** Forget `event.Skip()` in a `wxEVT_PAINT` handler.
- **Why?** If you don't `Skip()`, the default system behavior (like highlighting a button) might be blocked.

## Build Tools and Project Structure

**Feature 33: Build Tools**
- **MANDATORY:** Use CMake.
- **FORBIDDEN:** Manually maintain `.vcxproj` or Makefiles.
- **Why?** wxWidgets 3.3.x has vastly improved CMake support, making it the fastest way to link the library.

**Feature 34: PCH**
- **MANDATORY:** Use Precompiled Headers (`wx/wxprec.h`).
- **FORBIDDEN:** Include `<wx/wx.h>` in every single file.
- **Why?** Using wxprec.h can reduce compilation time by over 60% on large projects.

**Feature 35: Resources**
- **MANDATORY:** Use XRC (XML Resources) for UI.
- **FORBIDDEN:** Code every single wxButton placement in C++.
- **Why?** XRC separates your logic from your layout, allowing you to tweak the UI without recompiling.

## Key Principles

### Visualizing the Lifecycle

One of the biggest "Don'ts" is trying to manage the application lifecycle manually. wxWidgets uses a specific startup and shutdown sequence.

### The "Golden Rule" for 3.3.x

If you find yourself writing a Macro, stop and check if there is a Template alternative. Modern wxWidgets has replaced almost all the old macro-based logic with template-based logic that is easier for the compiler to optimize and easier for you to debug.

### Visualizing the UI Hierarchy

In wxWidgets, the relationship between windows is a tree. Understanding this helps you avoid manual memory management.

### Pro-Tip: The "Parent" Rule

When you create a control (like a `wxButton`), you pass a `this` pointer as the parent:
```cpp
new wxButton(this, wxID_ANY, "OK");
```

**The "Do":** Trust the parent. When you `Destroy()` the parent frame, wxWidgets automatically iterates through the children and deletes them properly. You don't need to track them yourself!

### Critical Lifecycle Diagram

If you find yourself wondering "where do I put my cleanup code?" or "why is my frame not showing?", refer to this order of operations.

### The "Golden Rule" for 2026

**Think "Standard C++" first.** In the old days (version 2.4 - 2.8), wxWidgets had to reinvent the wheel because C++ didn't have a standard library for strings, threads, or containers.

- **Today:** If you need a list, use `std::vector`.
- **Today:** If you need a thread, use `std::thread` (and `CallAfter` to talk to the UI).
- **Today:** Use `nullptr` instead of `NULL` or `0`.