## 2025-02-18 - Rendering Bottleneck: Per-Tile Logic
Learning: `TileRenderer::DrawTile` executes per visible tile every frame. Logic placed here, like string formatting for tooltips, multiplies O(ViewSize * TileDepth), causing massive CPU overhead even for static scenes.
Action: Move per-frame logic that only applies to the hovered tile (like tooltips) behind a `map_x == mouse_x` check or refactor to a separate pass.
