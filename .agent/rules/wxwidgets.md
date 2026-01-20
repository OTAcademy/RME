---
description: wxWidgets UI Best Practices for RME Map Editor
trigger: always_on
---

# wxWidgets Architecture & UI Rules

## 🎯 Core Principles

### 1. Responsive Layouts - wxWrapSizer is MANDATORY

**For ALL tileset/brush grids:**
```cpp
// ✅ CORRECT - Responsive wrapping layout
wxWrapSizer* sizer = new wxWrapSizer(wxHORIZONTAL);
for (const auto& brush : brushes) {
    auto* panel = CreateBrushButton(brush);
    sizer->Add(panel, 0, wxALL, 2);
}
SetSizer(sizer);

// ❌ WRONG - Fixed grid that doesn't adapt to window size
wxGridSizer* grid = new wxGridSizer(4, 4, 5, 5);  // BANNED for tilesets!
wxFlexGridSizer* flex = new wxFlexGridSizer(4);   // BANNED for tilesets!
```

### 2. Virtual List Controls for Large Data

**When displaying 100+ items:**
```cpp
// ✅ CORRECT - Virtual mode for performance
class ItemListCtrl : public wxListCtrl {
    std::vector<Item>& m_items;
public:
    ItemListCtrl(wxWindow* parent, std::vector<Item>& items)
        : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxLC_REPORT | wxLC_VIRTUAL) {
        SetItemCount(items.size());
    }
    wxString OnGetItemText(long item, long column) const override {
        return m_items[item].GetColumnText(column);
    }
};

// ❌ WRONG - Adding items one by one
for (const auto& item : thousands_of_items) {
    list->InsertItem(list->GetItemCount(), item.name);  // SLOW!
}
```

### 3. Modern Event Binding

```cpp
// ✅ CORRECT - Bind() with lambdas
Bind(wxEVT_BUTTON, &MyWindow::OnOK, this, wxID_OK);
Bind(wxEVT_SIZE, [this](wxSizeEvent& e) {
    RecalculateLayout();
    e.Skip();
});

// ✅ CORRECT - Bind with ID range
Bind(wxEVT_MENU, &MyWindow::OnBrushSelected, this, ID_BRUSH_FIRST, ID_BRUSH_LAST);

// ❌ AVOID in new code - Event tables
BEGIN_EVENT_TABLE(MyWindow, wxFrame)
    EVT_BUTTON(wxID_OK, MyWindow::OnOK)  // Legacy pattern
END_EVENT_TABLE()
```

---

## 🏗️ Window Architecture

### Separation of Concerns

```
┌─────────────────────────────────────────────────┐
│                    MainFrame                     │
│  (wxFrame - Window management only)              │
├─────────────────────────────────────────────────┤
│    Palette Panels    │     Map Canvas           │
│   (wxPanel + sizers) │  (wxGLCanvas + OpenGL)   │
├─────────────────────────────────────────────────┤
│              Editor Controller                   │
│        (Business logic - NO UI code)             │
└─────────────────────────────────────────────────┘
```

### Window vs Panel Rules

| Type | Use For | Contains |
|------|---------|----------|
| `wxFrame` | Top-level windows | Menu bar, status bar, main layout |
| `wxDialog` | Modal interactions | Form controls, OK/Cancel buttons |
| `wxPanel` | Reusable UI sections | Sizers set by OWNER, not panel |
| Custom Control | Specialized widgets | Self-contained, minimal dependencies |

---

## 🔧 Sizer Rules

### Always Use Sizers
```cpp
// ✅ CORRECT - Sizer manages layout
wxBoxSizer* main = new wxBoxSizer(wxVERTICAL);
main->Add(toolbar, 0, wxEXPAND);
main->Add(content, 1, wxEXPAND | wxALL, 5);
main->Add(statusbar, 0, wxEXPAND);
SetSizer(main);

// ❌ WRONG - Absolute positioning
button->SetPosition(wxPoint(100, 200));  // BANNED!
button->SetSize(80, 25);                  // BANNED!
```

### Sizer Selection Guide

| Sizer | Use Case |
|-------|----------|
| `wxBoxSizer` | Linear layouts (toolbars, form rows) |
| `wxWrapSizer` | **Tileset grids** (MANDATORY) |
| `wxFlexGridSizer` | True data tables only |
| `wxGridBagSizer` | Complex forms with spanning |
| `wxStaticBoxSizer` | Grouped related controls |

---

## ⚡ Performance Patterns

### Freeze During Bulk Updates
```cpp
// ✅ CORRECT - Freeze prevents flicker
Freeze();
for (const auto& item : items) {
    AddListItem(item);
}
Thaw();

// ❌ WRONG - Causes visual flicker
for (const auto& item : items) {
    AddListItem(item);  // Repaints on each add!
}
```

### Thread Safety
```cpp
// ✅ CORRECT - CallAfter for UI from worker thread
wxGetApp().CallAfter([this, msg = std::move(message)]() {
    m_statusBar->SetStatusText(msg);
});

// ❌ WRONG - Direct UI from worker thread
m_statusBar->SetStatusText(message);  // UNDEFINED BEHAVIOR!
```

---

## 📜 Validation Checklist

Before submitting wxWidgets UI code:
```
☐ Tileset layouts use wxWrapSizer
☐ Large lists use virtual mode
☐ All layouts use sizers (no absolute positioning)
☐ Events use Bind() (not event tables for new code)
☐ Freeze/Thaw around bulk updates
☐ UI updates from main thread only or via CallAfter
☐ Panels don't set their own sizers (owner does)
```