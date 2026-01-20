---
description: C++20 Coding Standards and SOLID Architecture Rules
---

# C++20 Style & SOLID Architecture

## 🎯 SOLID Principles - THE BIBLE

### S - Single Responsibility Principle
```
Every class/function does ONE thing well.

VIOLATIONS (trigger immediate refactoring):
☐ File > 500 lines → SPLIT
☐ Function > 50 lines → SPLIT
☐ Class with "and" in description → SPLIT
```

### O - Open/Closed Principle
```cpp
// ✅ Open for extension, closed for modification
class Brush {
    virtual void draw(Tile* tile) = 0;  // Extend via inheritance
};

// ❌ Switch statements that grow with each new type
switch (brushType) {  // Violates OCP
    case GROUND: ...
    case WALL: ...
    // Adding new brush = modifying this code
}
```

### L - Liskov Substitution
```cpp
// ✅ Derived classes respect base contracts
class GroundBrush : public Brush {
    void draw(Tile* tile) override;  // Fully substitutable
};
```

### I - Interface Segregation
```cpp
// ✅ Small, focused interfaces
class IDrawable { virtual void draw() = 0; };
class ISerializable { virtual void save() = 0; };

// ❌ Fat interfaces
class IEverything {
    virtual void draw() = 0;
    virtual void save() = 0;
    virtual void load() = 0;
    virtual void network() = 0;  // Forces unused implementations
};
```

### D - Dependency Inversion
```cpp
// ✅ Depend on abstractions
class Editor {
    std::unique_ptr<IMap> m_map;  // Interface, not concrete
public:
    Editor(std::unique_ptr<IMap> map) : m_map(std::move(map)) {}
};

// ❌ Depend on concrete implementations
class Editor {
    OTBMMap m_map;  // Concrete type = hard to test/extend
};
```

---

## 💻 C++20 Features to Use

### std::format (Not printf/sprintf)
```cpp
// ✅ Modern formatting
std::string msg = std::format("Position: ({}, {}, {})", x, y, z);

// ❌ C-style (legacy only)
char buf[256];
sprintf(buf, "Position: (%d, %d, %d)", x, y, z);
```

### Concepts for Templates
```cpp
// ✅ Constrained templates
template<typename T>
requires std::integral<T>
T snap_to_grid(T value, T grid_size) {
    return (value / grid_size) * grid_size;
}

// ✅ Concept for map coordinates
template<typename T>
concept Coordinate = requires(T t) {
    { t.x } -> std::convertible_to<int>;
    { t.y } -> std::convertible_to<int>;
    { t.z } -> std::convertible_to<int>;
};
```

### std::span for Array Views
```cpp
// ✅ Non-owning view
void process_tiles(std::span<Tile*> tiles) {
    for (auto* tile : tiles) { ... }
}

// ❌ Old-style pointer + size
void process_tiles(Tile** tiles, size_t count);
```

### Ranges
```cpp
// ✅ Ranges pipelines
auto visible_tiles = tiles
    | std::views::filter([](auto& t) { return t.isVisible(); })
    | std::views::take(100);

// ❌ Manual loops for simple transforms
std::vector<int> ids;
for (auto& t : tiles) {
    if (t.isVisible()) ids.push_back(t.id);
}
```

### Designated Initializers
```cpp
// ✅ Clear struct initialization
Position pos{.x = 100, .y = 200, .z = 7};

// ❌ Positional (unclear)
Position pos{100, 200, 7};
```

---

## 🔧 Memory Management

### Smart Pointer Rules
```cpp
// ✅ Unique ownership
std::unique_ptr<Tile> tile = std::make_unique<Tile>(...);

// ✅ Shared when truly shared
std::shared_ptr<Texture> texture;  // Multiple sprites use same texture

// ✅ Raw pointer = observation only
void inspect(const Tile* tile);  // Does NOT own

// ❌ BANNED
Tile* t = new Tile();
delete t;
```

### RAII for Resources
```cpp
// ✅ RAII wrapper for OpenGL
class VertexBuffer {
    GLuint m_id = 0;
public:
    VertexBuffer() { glGenBuffers(1, &m_id); }
    ~VertexBuffer() { if (m_id) glDeleteBuffers(1, &m_id); }
    
    // Non-copyable, moveable
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& o) noexcept : m_id(o.m_id) { o.m_id = 0; }
};
```

---

## 📝 Incremental Improvements

When touching existing code, apply these fixes:

| Old Pattern | Modern Pattern |
|-------------|----------------|
| `NULL` | `nullptr` |
| `0` for pointer | `nullptr` |
| `typedef` | `using` |
| `const char*` params | `std::string_view` |
| Iterator loops | Range-based for |
| `virtual ~Base() {}` | `virtual ~Base() = default;` |
| Missing `override` | Add `override` |
| C-style casts | `static_cast`, `dynamic_cast` |

---

## 📜 Validation Checklist

```
☐ SOLID principles followed
☐ Functions < 50 lines
☐ Files < 500 lines
☐ Smart pointers for ownership
☐ Raw pointers for observation only
☐ RAII for all resources
☐ C++20 features where applicable
☐ `nullptr` not `NULL`
☐ `override` on all virtual overrides
```
