# RME Codebase Encyclopedia

> **Remere's Map Editor (RME)** - A tile-based map editor for Open Tibia servers.
> Built with C++20, wxWidgets GUI, OpenGL 4.6 rendering, NanoVG overlays.

---

## Quick Reference

| Task | Where to Look |
|------|---------------|
| Map rendering | `rendering/map_drawer.cpp`, `rendering/core/graphics.cpp` |
| Add brush type | `brushes/brush.h` (base), `brushes/[type]/` directories |
| File I/O | `io/iomap_otbm.cpp`, `io/iomap_otmm.cpp` |
| Client data loading | `io/loaders/dat_loader.cpp`, `io/loaders/spr_loader.cpp` |
| Networking/Live | `live/live_*.cpp`, `net/net_connection.cpp` |
| UI windows/dialogs | `ui/dialogs/`, `ui/properties/`, `ui/map/` |
| Undo/redo | `editor/action.cpp`, `editor/action_queue.cpp` |
| Editor operations | `editor/operations/draw_operations.cpp`, `selection_operations.cpp` |
| Item properties | `game/item.h`, `game/items.cpp`, `game/item_attributes.cpp` |
| Palette system | `palette/palette_window.cpp`, `palette/panels/` |
| Toolbar system | `ui/toolbar/toolbar_registry.cpp`, `ui/toolbar/*_toolbar.cpp` |
| Replace tool | `ui/replace_tool/replace_tool_window.cpp` |
| In-game preview | `ingame_preview/ingame_preview_window.cpp` |
| NanoVG/custom controls | `util/nanovg_canvas.cpp`, `util/virtual_item_grid.cpp` |

---

## Architecture Overview

```
┌──────────────────────────────────────────────────────────────────────────┐
│                         Application Layer                                │
│  application.cpp, gui.cpp, main_frame.cpp, main_menubar.cpp              │
├──────────────────────────────────────────────────────────────────────────┤
│                    UI Layer (wxWidgets + NanoVG)                          │
│  Palette | Toolbar | Properties | Dialogs | Replace Tool | Controls      │
├──────────────────────────────────────────────────────────────────────────┤
│               Rendering Layer (OpenGL 4.6 + NanoVG)                      │
│  MapDrawer → SpriteBatch → TextureAtlas → ShaderProgram                  │
│  Drawers: tiles, entities, overlays, cursors | PostProcess               │
│  Controllers: drawing, selection, zoom, navigation, tooltips             │
├────────────────┬───────────────────┬─────────────────────────────────────┤
│  Editor Core   │   Brush System    │    Map Data Layer                    │
│  editor.cpp    │  brush.h + impls  │  map.cpp, basemap.cpp, tile.cpp     │
│  action.cpp    │  15 brush types   │  spatial_hash_grid.h                 │
│  operations/*  │  managers/preview │  map_region.cpp                      │
├────────────────┴───────────────────┴─────────────────────────────────────┤
│                      Game Data Layer                                     │
│  item.cpp, creature.cpp, house.cpp, spawn.cpp, town.cpp, materials.cpp   │
├──────────────────────────────────────────────────────────────────────────┤
│                         I/O Layer                                        │
│  iomap_otbm.cpp | iomap_otmm.cpp | filehandle.cpp | loaders/*            │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Core Systems

### Application & GUI
| File | Purpose |
|------|---------|
| `app/application.cpp` | Entry point, wxApp subclass, startup/shutdown |
| `ui/gui.cpp/h` | Central GUI controller, manages palettes/windows |
| `ui/main_frame.cpp` | Main application window |
| `ui/main_menubar.cpp` | Menu bar with actions & shortcuts |
| `ui/main_toolbar.cpp` | Primary toolbar |
| `ui/menubar/` | Decomposed menu action handlers (7 files) |
| `ui/toolbar/` | Modular toolbar system (9 toolbar types) |
| `ui/managers/` | UI lifecycle managers (8 types) |
| `app/settings.cpp` | Configuration persistence |
| `app/preferences.cpp` | Preferences dialog |

### Map System
| File | Purpose |
|------|---------|
| `map/map.cpp/h` | Map container, width/height, spawns, houses |
| `map/basemap.cpp/h` | QuadTree-based tile storage, tile access |
| `map/tile.cpp/h` | Tile data: ground, items, creature, spawn |
| `map/tile_operations.*` | Tile manipulation utilities |
| `map/map_region.cpp` | Region-based spatial partitioning |
| `map/spatial_hash_grid.h` | Hash-grid spatial index for viewport queries |
| `map/map_allocator.h` | Memory allocation pools |
| `map/map_statistics.*` | Map content analysis |
| `map/otml.*` | OTML format parser |

### Editor Core
| File | Purpose |
|------|---------|
| `editor/editor.cpp/h` | **Central controller**: orchestrates editing |
| `editor/action.cpp/h` | Undo/redo action system |
| `editor/action_queue.*` | Action stack management |
| `editor/selection.cpp/h` | Selected tiles management |
| `editor/copybuffer.cpp/h` | Copy/paste functionality |
| `editor/editor_tabs.*` | Multi-map tab control |
| `editor/hotkey_manager.*` | Keyboard shortcut management |
| `editor/operations/` | Discrete operations (draw, select, copy, search, clean) |
| `editor/persistence/` | Editor state save/load, tileset export |

### Rendering Pipeline
| File | Purpose |
|------|---------|
| `rendering/map_drawer.*` | Top-level render orchestrator |
| `rendering/core/sprite_batch.*` | Batched sprite rendering |
| `rendering/core/shader_program.*` | GLSL shader management |
| `rendering/core/texture_atlas.*` | Sprite atlas management |
| `rendering/core/render_view.*` | Camera/viewport state |
| `rendering/core/game_sprite.*` | Game sprite loading |
| `rendering/drawers/tiles/` | Floor, tile, shade rendering |
| `rendering/drawers/entities/` | Item, creature, sprite rendering |
| `rendering/drawers/overlays/` | Selection, grid, brush, preview, marker |
| `rendering/drawers/cursors/` | Brush cursor, drag shadow, live cursor |
| `rendering/ui/map_display.*` | **MapCanvas**: OpenGL canvas widget |
| `rendering/ui/tooltip_drawer.*` | NanoVG tooltip rendering |
| `rendering/utilities/light_drawer.*` | Light overlay rendering |
| `rendering/postprocess/` | Post-processing effects (scanline, xBRZ) |

---

## Brush System

Base class: `Brush` in `brushes/brush.h`

### Brush Types (15)
| Brush | Directory | Purpose |
|-------|-----------|---------|
| `GroundBrush` | `brushes/ground/` | Terrain with auto-bordering |
| `WallBrush` | `brushes/wall/` | Walls with corner/door logic |
| `DoodadBrush` | `brushes/doodad/` | Decorative objects |
| `CarpetBrush` | `brushes/carpet/` | Carpets with edges |
| `TableBrush` | `brushes/table/` | Tables with adjacency |
| `CreatureBrush` | `brushes/creature/` | Place creatures |
| `SpawnBrush` | `brushes/spawn/` | Spawn zones |
| `HouseBrush` | `brushes/house/` | House tiles |
| `HouseExitBrush` | `brushes/house/` | House exits |
| `DoorBrush` | `brushes/door/` | Door placement |
| `WaypointBrush` | `brushes/waypoint/` | NPC waypoints |
| `FlagBrush` | `brushes/flag/` | Zone flags |
| `RAWBrush` | `brushes/raw/` | Raw item placement |
| `EraserBrush` | `brushes/eraser/` | Remove items |
| `OptionalBorderBrush` | `brushes/border/` | Optional borders |

### Brush Managers
- `BrushManager`: Global registry, `getBrush(name)`
- `AutoborderPreviewManager`: Real-time border preview
- `DoodadPreviewManager`: Doodad placement preview

---

## Data Model

### Tile (`map/tile.h`)
```cpp
class Tile {
    TileLocation* location;  // x, y, z position
    Item* ground;            // Ground item
    ItemVector items;        // Stacked items
    Creature* creature;      // One per tile
    Spawn* spawn;            // Spawn reference
    uint32_t house_id;       // House ownership
};
```

### Item (`game/item.h`)
```cpp
class Item {
    uint16_t id;          // Item type ID
    uint16_t subtype;     // Count/charges/fluid
    bool selected;
    ItemAttributes* attributes;  // Dynamic attrs
    // Factory: Item::Create(id, subtype)
};
```

---

## I/O System

### Map Formats
| Format | Files | Description |
|--------|-------|-------------|
| OTBM | `io/iomap_otbm.*` | Binary map format (primary) |
| OTMM | `io/iomap_otmm.*` | Alternative format |

### Client Data Loaders
| Loader | Purpose |
|--------|---------|
| `dat_loader.*` | Client .dat appearances |
| `spr_loader.*` | Client .spr sprite data |
| `otfi_loader.*` | OTFI metadata |

---

## Networking (Live Editing)

Real-time collaborative editing via TCP (Boost.ASIO):
- `live_server.*` / `live_client.*` / `live_peer.*`
- `live_socket.*` / `live_packets.h`
- `live_action.*` — action synchronization
- `live_manager.*` — session lifecycle

---

## In-Game Preview

Separate OpenGL window showing game-accurate rendering:
- `ingame_preview_window.*` — top-level window
- `ingame_preview_canvas.*` — OpenGL canvas with player movement
- `ingame_preview_renderer.*` — tile/creature rendering
- `floor_visibility_calculator.*` — multi-floor visibility

---

## Global Singletons

| Variable | Type | Purpose |
|----------|------|---------|
| `g_gui` | `GUI` | Central GUI controller |
| `g_brushes` | `Brushes` | Brush manager |
| `g_items` | `Items` | Item type database |
| `g_creatures` | `Creatures` | Creature database |
| `g_materials` | `Materials` | Materials/tilesets |
| `g_settings` | `Settings` | Configuration |
| `g_graphics` | `Graphics` | Sprite/texture management |

---

## File Naming Conventions

| Pattern | Meaning |
|---------|---------|
| `*_window.cpp` | Dialog/window class |
| `*_brush.cpp` | Brush implementation |
| `*_drawer.cpp` | Rendering drawer |
| `*_controller.cpp` | Input/interaction controller |
| `*_handler.cpp` | Event/action handler |
| `*_manager.cpp` | Lifecycle/state manager |
| `*_service.cpp` | Stateless service |
| `io*.cpp` | I/O handlers |
| `palette_*.cpp` | Palette UI components |
| `live_*.cpp` | Networking components |

---

## External Dependencies

| Library | Purpose | Integration |
|---------|---------|-------------|
| wxWidgets | GUI framework | Core UI |
| OpenGL 4.6 | Rendering | Map display, SpriteBatch |
| NanoVG | 2D vector graphics | Tooltips, HUD, custom controls |
| GLAD | GL function loading | `glad.h` |
| GLM | Math library | Transforms, vectors |
| Boost.Asio | Networking | Live editing |
| nlohmann_json | JSON parsing | Config files |
| libarchive | Archive handling | OTGZ loading |
| pugixml | XML parsing | Materials/configs |
| spdlog | Logging | Application logging |
