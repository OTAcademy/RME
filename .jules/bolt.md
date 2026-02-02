## 2025-02-18 - Rendering Bottleneck: Per-Tile Logic
Learning: `TileRenderer::DrawTile` executes per visible tile every frame. Tooltips are currently drawn for all items, which is intentional (labels), but this design choice inherently limits performance on large views due to string operations.
Action: Future optimizations for tooltips should focus on caching the generated strings or using a more efficient font renderer, rather than culling them, as culling contradicts the "label" behavior.

## 2025-02-18 - Tooltip Allocations
Learning: `TooltipDrawer::draw` was allocating `std::vector<FieldLine>` inside a loop for every tooltip, causing allocator churn. Also `TileRenderer` was deep-copying `TooltipData` (containing strings/vectors) every frame.
Action: Use persistent scratch buffers (member variables) for immediate-mode rendering loops to avoid allocation. Use `std::move` for passing heavy transient data like tooltips.
