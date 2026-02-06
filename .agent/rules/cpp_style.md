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

**Feature 1: Modules (import)**
- **Syntax:** `import std; import my.tile.module;`
- **Use Case:** Drastically faster compile times; no more header guard mess.

**Feature 2: Concepts**
- **Syntax:** `template<TileType T> void process(T tile);`
- **Use Case:** Constrain template types (e.g., `is_tile_type`, `is_plugin`).

**Feature 3: Ranges (std::views)**
- **Syntax:** `tiles | std::views::filter(isActive) | std::views::transform(getId);`
- **Use Case:** Filter/transform tiles in-place without creating temporary vectors.

**Feature 4: std::span**
- **Syntax:** `void processTiles(std::span<Tile> tiles);`
- **Use Case:** Pass slices of map arrays safely without copying memory.

**Feature 5: std::format**
- **Syntax:** `std::format("Tile at ({}, {})", x, y);`
- **Use Case:** Type-safe, fast string formatting for logs and UI text.

**Feature 6: Designated Initializers**
- **Syntax:** `Tile{ .id=1, .x=10, .y=20 };`
- **Use Case:** Clearer tile/object creation.

**Feature 7: Coroutines**
- **Syntax:** `Task<Texture> loadAsync() { co_return co_await loader.fetch(); }`
- **Use Case:** Async asset loading or step-by-step pathfinding visualizers.

**Feature 8: consteval**
- **Syntax:** `consteval int hash(const char* str) { ... }`
- **Use Case:** Force functions (like ID hashing) to run entirely at compile time.

**Feature 9: Three-way Comparison (<=>)**
- **Syntax:** `auto operator<=>(const Tile&) const = default;`
- **Use Case:** Default "spaceship" operator for easy sorting of tile layers.

**Feature 10: std::atomic_ref**
- **Syntax:** `std::atomic_ref<int> ref(tile.count); ref++;`
- **Use Case:** Perform thread-safe updates on existing non-atomic tile data.

**Feature 11: std::jthread**
- **Syntax:** `std::jthread worker([](std::stop_token st) { ... });`
- **Use Case:** Threads that automatically join on destruction; safer for workers.

**Feature 12: std::bit_cast**
- **Syntax:** `auto id = std::bit_cast<uint32_t>(bytes);`
- **Use Case:** Safe type punning (e.g., converting raw byte buffers to tile IDs).

**Feature 27: Structured Binding in init**
- **Syntax:** `if (auto [it, inserted] = map.insert({key, val}); inserted) { ... }`
- **Use Case:** Combine declaration and condition checking.

**Feature 28: std::stop_token**
- **Syntax:** `void export_map(std::stop_token st) { if(st.stop_requested()) return; }`
- **Use Case:** Gracefully cancel long-running tasks like map exports.

**Feature 29: [[nodiscard]] with message**
- **Syntax:** `[[nodiscard("Check if save succeeded!")]] bool save();`
- **Use Case:** Warn developers if they ignore critical return values (like `save()`).

**Feature 31: std::source_location**
- **Syntax:** `void log(std::source_location loc = std::source_location::current());`
- **Use Case:** Log errors with file/line info automatically without using macros.

**Feature 32: std::numbers**
- **Syntax:** `double angle = std::numbers::pi / 4;`
- **Use Case:** Built-in constants like `std::numbers::pi` for rotation and math logic.

**Feature 33: Constexpr std::vector**
- **Syntax:** `constexpr std::vector<int> lookup = {1, 2, 3, 4};`
- **Use Case:** Create and sort small lookup tables or brush shapes at compile time.

**Feature 34: std::lerp**
- **Syntax:** `float pos = std::lerp(start, end, 0.5f);`
- **Use Case:** Smoothly interpolate camera movement or UI transitions.

**Feature 35: std::midpoint**
- **Syntax:** `int center = std::midpoint(x1, x2);`
- **Use Case:** Safe calculation of midpoints between tile coordinates (prevents overflow).

**Feature 36: Non-type Template Parameters**
- **Syntax:** `template<auto Name> class Brush; Brush<"Grass"> grass;`
- **Use Case:** Use strings or floats as template arguments.

**Feature 37: Lambda Capture Init**
- **Syntax:** `[ptr = std::move(unique_ptr)]() { ptr->use(); }`
- **Use Case:** Move-only types (like `std::unique_ptr`) into UI event lambdas.

**Feature 38: std::identity**
- **Syntax:** `std::ranges::sort(tiles, {}, std::identity{});`
- **Use Case:** A standard "do nothing" function used for projection in ranges/algorithms.

## C++23 Features

**Feature 13: Multi-dimensional operator[]**
- **Syntax:** `auto& tile = map[x, y, layer];`
- **Use Case:** Access map layers directly with clean syntax.

**Feature 14: std::expected**
- **Syntax:** `std::expected<Map, Error> loadMap(path);`
- **Use Case:** Handle file I/O or script errors without exceptions.

**Feature 15: std::flat_map**
- **Syntax:** `std::flat_map<int, TileProps> properties;`
- **Use Case:** Cache-friendly, vector-backed maps for tile properties.

**Feature 16: std::print**
- **Syntax:** `std::print("Position: {}, {}\n", x, y);`
- **Use Case:** Cleaner, faster alternative to `std::cout` for debug consoles.

**Feature 17: Deducing this**
- **Syntax:** `void method(this auto&& self) { self.process(); }`
- **Use Case:** Simplifies CRTP for GUI component hierarchies and callbacks.

**Feature 18: std::mdspan**
- **Syntax:** `std::mdspan<Tile, std::dextents<int, 3>> grid(data, x, y, z);`
- **Use Case:** View 1D vectors as 2D/3D grids without memory overhead.

**Feature 19: std::optional extensions**
- **Syntax:** `opt.and_then(process).or_else(handleError).value_or(default);`
- **Use Case:** Monadic operations for cleaner logic flows.

**Feature 20: if consteval**
- **Syntax:** `if consteval { /* compile-time */ } else { /* runtime */ }`
- **Use Case:** Optimize code differently for compile-time vs. run-time.

**Feature 21: std::stacktrace**
- **Syntax:** `auto trace = std::stacktrace::current();`
- **Use Case:** Automatically capture exactly where a map crash occurred.

**Feature 22: std::string::contains**
- **Syntax:** `if (filename.contains(".otbm")) { ... }`
- **Use Case:** Simple utility for searching asset names or tags.

**Feature 30: Attributes on Lambda**
- **Syntax:** `auto f = [[nodiscard]] []() -> int { return 42; };`
- **Use Case:** Ability to use `[[nodiscard]]` or `[[deprecated]]` on lambdas.

**Feature 39: std::ranges::contains**
- **Syntax:** `if (std::ranges::contains(tileIDs, targetID)) { ... }`
- **Use Case:** Quickly check if a tile list contains a specific ID without `std::find`.

**Feature 40: std::generator**
- **Syntax:** `std::generator<Tile*> neighbors() { co_yield tile; }`
- **Use Case:** Use `co_yield` to iterate through complex map structures (like a spiral search).

**Feature 41: Monadic std::optional**
- **Syntax:** `findTile(x,y).and_then(getProp).value_or(defaultProp);`
- **Use Case:** Chain tile lookups cleanly.

**Feature 42: std::byteswap**
- **Syntax:** `uint32_t swapped = std::byteswap(value);`
- **Use Case:** Fast endian conversion when loading old RME/OTBM map files.

**Feature 43: std::to_underlying**
- **Syntax:** `int val = std::to_underlying(TileColor::Red);`
- **Use Case:** Convert enum class (like `TileColor`) to its integer type safely.

**Feature 44: std::move_only_function**
- **Syntax:** `std::move_only_function<void()> callback = [ptr = std::move(p)](){};`
- **Use Case:** Better for task queues where handlers shouldn't be copied.

**Feature 45: Static operator()**
- **Syntax:** `struct Functor { static int operator()(int x) { return x * 2; } };`
- **Use Case:** Optimize lambdas/functors that don't capture state for the compiler.

**Feature 46: std::unreachable()**
- **Syntax:** `default: std::unreachable();`
- **Use Case:** Tells the compiler a code path (like an exhaustive switch) is impossible.

## C++26 Features (Upcoming)

**Feature 23: Placeholder _**
- **Syntax:** `auto [x, y, _] = getTileCoords();`
- **Use Case:** Ignore unused variables in structured bindings.

**Feature 24: std::copyable_function**
- **Syntax:** `std::copyable_function<void()> handler = onClick;`
- **Use Case:** Better replacement for `std::function` for event handlers.

**Feature 25: String interpolation (v.2)**
- **Syntax:** `auto msg = f"Tile {id} at position ({x}, {y})";`
- **Use Case:** Further improvements to how UI strings are handled.

**Feature 26: Pack Indexing**
- **Syntax:** `template<size_t I, typename... Ts> using At = Ts...[I];`
- **Use Case:** Easier manipulation of variadic templates for plugin systems.

**Feature 47: Reflection (Static)**
- **Syntax:** `for (auto [name, value] : std::meta::members_of(^Tile)) { ... }`
- **Use Case:** Automatically generate UI properties for a `struct Tile`. (Highly anticipated)

**Feature 48: Erroneous Behavior Tracking**
- **Syntax:** `[[indeterminate]] int uninitialized;`
- **Use Case:** Catch bugs where "uninitialized" tile memory is accessed by mistake.

**Feature 49: Attributes on Structured Bindings**
- **Syntax:** `auto [x, y, [[maybe_unused]] z] = getPos3D();`
- **Use Case:** Apply `[[maybe_unused]]` to specific parts of a returned coordinate tuple.

**Feature 50: std::not_fn improvements**
- **Syntax:** `auto notEmpty = std::not_fn([](auto& x) { return x.empty(); });`
- **Use Case:** Negate complex predicates when filtering map objects.

---

## Architectural Highlights

### std::mdspan and operator[] (C++23)

In the old RME (C++11), you likely accessed tiles via a 1D vector using `tiles[y * width + x]`. This is error-prone and hard to read.

With C++23, you can use `std::mdspan` to wrap a simple `std::vector` and treat it as a 3D volume (X, Y, Z/Layer). This gives you the performance of a flat array with the syntax of a multidimensional one.

**Example:**
```cpp
std::vector<Tile> tiles(width * height * layers);
std::mdspan map_view(tiles.data(), width, height, layers);

// Clean access:
auto& tile = map_view[x, y, z];
```

---

## Implementation Focus

### The Power of std::generator (C++23)

One of the clunkiest parts of RME is the nested loops for "Flood Fill" or "Auto-bordering." In C++11, you had to manage state manually if you wanted to pause an algorithm or visualize it.

With C++23 Generators, you can write a "lazy" algorithm that yields coordinates one by one. This is perfect for a "Step-by-Step" debugger in your editor.

**Example:**
```cpp
// C++23: A generator that yields neighbors only when needed
std::generator<Tile*> get_neighbors(int x, int y) {
    for (int dx : {-1, 0, 1}) {
        for (int dy : {-1, 0, 1}) {
            if (auto* t = map.getTile(x + dx, y + dy)) 
                co_yield t;
        }
    }
}

// Usage
for (Tile* t : get_neighbors(100, 100)) {
    t->highlight(); // Easy to read, no manual iterators
}
```

### Memory Efficiency with std::mdspan (C++23)

Since RME deals with multiple floors (Z-axis), `std::mdspan` allows you to treat a flat `std::vector<Tile>` as a 3D block. This keeps all your data in one contiguous memory chunk (great for the CPU cache) while letting you access it via `map[x, y, z]`.

---

## Comparison: RME Then vs. Now

### RME (C++11)
- Used `std::set` or `std::map` for tile items
- Caused massive memory fragmentation
- Slow "Save Map" times due to pointer chasing

### Your Editor (C++23/26)
- Uses `std::flat_map` and `std::mdspan`
- Data is packed tightly
- Saving a map is basically a single `fwrite` of a contiguous buffer

---

## Quick Reference by Standard

### C++20 (19 features)
Features 1-12, 27-29, 31-38

### C++23 (22 features)
Features 13-22, 30, 39-46

### C++26 (9 features)
Features 23-26, 47-50

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
