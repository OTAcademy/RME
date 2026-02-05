## 2025-02-18 - Rendering Bottleneck: Per-Tile Logic
Learning: `TileRenderer::DrawTile` executes per visible tile every frame. Tooltips are currently drawn for all items, which is intentional (labels), but this design choice inherently limits performance on large views due to string operations.
Action: Future optimizations for tooltips should focus on caching the generated strings or using a more efficient font renderer, rather than culling them, as culling contradicts the "label" behavior.

## 2025-02-18 - Tooltip Allocations
Learning: `TooltipDrawer::draw` was allocating `std::vector<FieldLine>` inside a loop for every tooltip, causing allocator churn. Also `TileRenderer` was deep-copying `TooltipData` (containing strings/vectors) every frame.
Action: Use persistent scratch buffers (member variables) for immediate-mode rendering loops to avoid allocation. Use `std::move` for passing heavy transient data like tooltips.

## 2025-02-18 - Tooltip String Allocations
Learning: `CreateItemTooltipData` was allocating thousands of `std::string` objects (from `std::string_view`) every frame for item names. By pooling `TooltipData` objects and clearing them (which keeps `std::string` capacity), we eliminate these allocations entirely after warmup.
Action: Prefer modifying pooled objects in-place (`FillItemTooltipData`) over returning new objects by value in hot paths.

## 2025-02-18 - Minimap Color Caching
Learning: `Tile::update` failed to reset `minimapColor` when no color was found, leading to stale colors if items were removed. Also `deepCopy` produced `INVALID_MINIMAP_COLOR` tiles, forcing `getMiniMapColor` to scan items every time.
Action: Always initialize/reset cached values in `update()` methods to ensure state validity. Use `minimapColor = 0` (or valid "empty" value) instead of `INVALID` to enable unconditional fast-path access.

## 2026-02-05 - Sprite Rendering Loop Overhead
Learning: `ItemDrawer::BlitItem` was running a triple nested loop for every item, even though 99% of items are 1x1x1 single sprites. This caused significant branch prediction overhead per item.
Action: Check for common trivial cases (1x1x1) early and use a fast path to bypass loops entirely.

## 2026-02-05 - Spatial Color Caching
Learning: `TileColorCalculator::GetHouseColor` used an `unordered_map` lookup for every tile. Since tiles are rendered in spatial order, adjacent tiles almost always share the same house ID.
Action: Use a 1-element `thread_local` cache (last input -> last output) to eliminate hash lookups for spatially coherent data sequences.

## 2026-02-05 - Tooltip String Allocations
Learning: `TooltipData` was performing deep copies of strings (`itemName`, `text`, etc.) from stable memory (`g_items`) into transient per-frame objects, causing thousands of allocations.
Action: Use `std::string_view` for transient DTOs that only live for the duration of a frame, especially when sourcing data from static or long-lived buffers.
