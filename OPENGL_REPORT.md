OPENGL RENDERING SPECIALIST - Daily Report
Date: 2024-05-24 10:00
Files Scanned: 24
Review Time: 20 minutes
Quick Summary
Found 1 critical issue in TileRenderer.cpp / SpriteBatch.cpp - Interleaved renderer calls causing GL state corruption.
Estimated improvement: Prevents rendering artifacts and potential crashes.
Issue Count
CRITICAL: 1
HIGH: 2
MEDIUM: 1
LOW: 1

Issues Found
CRITICAL: Interleaved SpriteBatch and PrimitiveRenderer calls without state protection
Location: src/rendering/drawers/tiles/tile_renderer.cpp (DrawHookIndicator) and src/rendering/core/sprite_batch.cpp
Problem: `TileRenderer` calls `DrawHookIndicator` (using `PrimitiveRenderer`) inside the tile loop while `SpriteBatch` is active. `PrimitiveRenderer::flush` changes the active shader and VAO. `SpriteBatch::flush` (called later) assumes its shader/VAO are still bound but does not re-bind them, leading to drawing with the wrong shader state.
Impact: Rendering corruption, undefined behavior, potential driver crashes.
Fix: Modify `SpriteBatch::flush` to robustly bind its shader and VAO every time, handling external state changes.

HIGH: Redundant texture binding in tile loop
Location: src/rendering/drawers/tiles/tile_renderer.cpp
Problem: Although `SpriteBatch` batches draw calls, the interleaving with `PrimitiveRenderer` forces frequent flushes and state changes (Shader/VAO switches), negating batching benefits.
Impact: Increased CPU overhead due to excessive GL state changes.
Fix: Convert `DrawHookIndicator` and `BrushCursorDrawer` to use `SpriteBatch` exclusively where possible, or batch primitives separately.

HIGH: Static tile geometry uploaded every frame
Location: src/rendering/drawers/map_layer_drawer.cpp
Problem: Map geometry is mostly static but is rebuilt and uploaded to `SpriteBatch`'s dynamic buffer every frame.
Impact: Wastes PCIe bandwidth and CPU time.
Fix: Implement static VBO caching for map chunks (nodes) using `GL_STATIC_DRAW`, only updating when tiles change.

MEDIUM: Hook Indicator using PrimitiveRenderer
Location: src/rendering/drawers/entities/item_drawer.cpp
Problem: `DrawHookIndicator` uses `PrimitiveRenderer` to draw simple triangles. `SpriteBatch` cannot easily draw arbitrary triangles, forcing a renderer switch.
Impact: Breaks batching in the main render loop.
Fix: Add a "hook" sprite to the atlas or implement a way to draw simple geometric shapes in `SpriteBatch`.

LOW/INFO: LightDrawer uses CPU accumulation
Location: src/rendering/drawers/tiles/tile_renderer.cpp
Problem: `AddLight` accumulates lights in a CPU vector.
Impact: Minor CPU overhead.
Fix: Acceptable for current light counts.

Summary Stats
Most common issue: Renderer Interleaving (2 locations)
Cleanest file: src/rendering/core/graphics.cpp
Needs attention: src/rendering/core/sprite_batch.cpp (Robustness fix required)
Estimated total speedup: N/A (Stability fix)
