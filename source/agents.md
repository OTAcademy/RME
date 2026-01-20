# RME Codebase Encyclopedia

> **Remere's Map Editor (RME)** - A tile-based map editor for Open Tibia servers.
> Built with C++20, wxWidgets GUI, OpenGL rendering.

---

## Quick Reference

| Task | Where to Look |
|------|---------------|
| Map rendering | `map_drawer.cpp`, `graphics.cpp` |
| Add brush type | `brush.h` (base), `*_brush.cpp` patterns |
| File I/O | `iomap_otbm.cpp`, `iomap_otmm.cpp` |
| Networking/Live | `live_*.cpp`, `net_connection.cpp` |
| UI windows | `*_window.cpp`, `palette_*.cpp` |
| Undo/redo | `action.cpp`, `editor.cpp` |
| Item properties | `item.h`, `items.cpp`, `item_attributes.cpp` |

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  application.cpp, gui.cpp, main_menubar.cpp, main_toolbar.cpp│
├─────────────────────────────────────────────────────────────┤
│                      Editor Core                             │
│        editor.cpp (orchestrates map, selection, actions)     │
├──────────────────┬──────────────────┬───────────────────────┤
│   Map System     │   Brush System   │    Selection/Actions  │
│ map.cpp,basemap  │ brush.h + impls  │ selection.cpp,action  │
├──────────────────┴──────────────────┴───────────────────────┤
│                    Data Layer                                │
│  tile.cpp, item.cpp, creature.cpp, house.cpp, materials.cpp │
├─────────────────────────────────────────────────────────────┤
│                    I/O Layer                                 │
│     iomap_otbm.cpp, iomap_otmm.cpp, filehandle.cpp          │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Systems

### Application & GUI
| File | Purpose |
|------|---------|
| `application.cpp` | Entry point, wxApp subclass, startup/shutdown |
| `gui.cpp/h` | Central GUI controller, manages palettes/windows |
| `main_menubar.cpp` | Menu bar with all editor commands |
| `main_toolbar.cpp` | Toolbar with brush/tool buttons |
| `editor_tabs.cpp` | Tab control for multiple open maps |
| `settings.cpp` | Configuration persistence |

### Map System
| File | Purpose |
|------|---------|
| `map.cpp/h` | Map container, width/height, spawns, houses |
| `basemap.cpp/h` | Base class for map storage, tile access |
| `map_region.cpp` | Spatial partitioning for efficient lookup |
| `map_allocator.h` | Memory allocation for map tiles |
| `tile.cpp/h` | Tile data: ground, items, creature, spawn |

### Editor Core
| File | Purpose |
|------|---------|
| `editor.cpp/h` | **Central controller**: draw/undraw, selection ops |
| `action.cpp/h` | Undo/redo system, ActionQueue, BatchAction |
| `selection.cpp/h` | Selected tiles management |
| `copybuffer.cpp/h` | Copy/paste functionality |

### Rendering
| File | Purpose |
|------|---------|
| `graphics.cpp/h` | Sprite loading, texture management |
| `map_drawer.cpp` | OpenGL tile rendering |
| `map_display.cpp` | Map canvas widget, mouse/keyboard handling |
| `light_drawer.cpp` | Light effect rendering |
| `sprites.h` | Sprite definitions and constants |

---

## Brush System

Base class: `Brush` in `brush.h`

```cpp
class Brush {
    virtual void draw(BaseMap* map, Tile* tile, void* parameter) = 0;
    virtual std::string getName() const = 0;
    // Type checking: isGround(), isWall(), isDoodad(), etc.
    // Type casting: asGround(), asWall(), asDoodad(), etc.
};
```

### Brush Types
| Brush | File | Purpose |
|-------|------|---------|
| `GroundBrush` | `ground_brush.cpp` | Terrain tiles with auto-bordering |
| `WallBrush` | `wall_brush.cpp` | Walls with corner/door logic |
| `DoodadBrush` | `doodad_brush.cpp` | Decorative objects |
| `CarpetBrush` | `carpet_brush.cpp` | Carpets with edge detection |
| `TableBrush` | `table_brush.cpp` | Tables with adjacency |
| `CreatureBrush` | `creature_brush.cpp` | Place creatures |
| `SpawnBrush` | `spawn_brush.cpp` | Spawn zones |
| `HouseBrush` | `house_brush.cpp` | House tile assignment |
| `HouseExitBrush` | `house_exit_brush.cpp` | House exit markers |
| `WaypointBrush` | `waypoint_brush.cpp` | NPC waypoints |
| `RAWBrush` | `raw_brush.cpp` | Single item placement |
| `EraserBrush` | `eraser_brush.cpp` | Remove items |

### Global Manager
```cpp
extern Brushes g_brushes;  // Access all brushes
g_brushes.getBrush("grass");  // Get by name
```

---

## Data Model

### Tile (`tile.h`)
```cpp
class Tile {
    TileLocation* location;  // Position on map
    Item* ground;            // Ground item
    ItemVector items;        // Stacked items
    Creature* creature;      // Monster/NPC (one per tile)
    Spawn* spawn;            // Spawn zone reference
    uint32_t house_id;       // House ownership
};
```

### Item (`item.h`)
```cpp
class Item {
    uint16_t id;             // Item type ID
    uint16_t subtype;        // Count/charges/fluid
    bool selected;           // Selection state
    // Factory: Item::Create(id, subtype)
};
```

### ItemType (`items.h`)
```cpp
class ItemType {
    // Properties loaded from items.otb/dat
    bool blockSolid, blockProjectile, moveable;
    uint8_t miniMapColor;
    Brush* brush;  // Associated brush if any
};
extern Items g_items;  // Global item database
```

### Creature (`creature.h`)
```cpp
class Creature {
    std::string name;
    CreatureType* type;
    Outfit outfit;
    bool selected;
};
```

---

## I/O System

### Map Formats
| Format | Files | Description |
|--------|-------|-------------|
| OTBM | `iomap_otbm.cpp/h` | Binary map format (primary) |
| OTMM | `iomap_otmm.cpp/h` | XML map format (legacy) |

### File Handling
| File | Purpose |
|------|---------|
| `filehandle.cpp/h` | Binary read/write helpers |
| `client_version.cpp` | Client data version management |

### Load Flow
```
Application::OnInit() 
  → ClientVersion::loadVersions()
  → Materials::loadMaterials() 
  → Editor::loadMap()
    → IOMapOTBM::loadMap()
```

---

## UI Windows

### Dialogs
| File | Purpose |
|------|---------|
| `about_window.cpp` | About dialog |
| `preferences.cpp` | Settings dialog |
| `welcome_dialog.cpp` | Startup welcome screen |
| `properties_window.cpp` | Item/tile properties editor |
| `find_item_window.cpp` | Item search dialog |
| `replace_items_window.cpp` | Bulk item replacement |

### Palettes
| File | Purpose |
|------|---------|
| `palette_window.cpp` | Palette container |
| `palette_brushlist.cpp` | Brush selection list |
| `palette_common.cpp` | Shared palette components |
| `palette_creature.cpp` | Creature palette |
| `palette_house.cpp` | House management palette |
| `palette_waypoints.cpp` | Waypoint palette |

### Other Windows
| File | Purpose |
|------|---------|
| `minimap_window.cpp` | Minimap overview |
| `browse_tile_window.cpp` | Tile contents browser |
| `result_window.cpp` | Search results display |

---

## Networking (Live Editing)

Enables collaborative real-time map editing.

| File | Purpose |
|------|---------|
| `live_server.cpp/h` | Host a live editing session |
| `live_client.cpp/h` | Connect to live session |
| `live_peer.cpp/h` | Individual connection handler |
| `live_socket.cpp/h` | Socket wrapper (Boost.Asio) |
| `live_packets.h` | Network packet definitions |
| `live_action.cpp/h` | Networked action synchronization |
| `live_tab.cpp/h` | Live session UI tab |
| `net_connection.cpp` | Connection management |

---

## Key Concepts

### Undo/Redo System
```cpp
// In editor.cpp
ActionQueue* actionQueue;  // Stack of actions
editor.addAction(new Action(...));
actionQueue->undo();
actionQueue->redo();
```

### Selection
```cpp
// In selection.cpp
class Selection {
    TileSet tiles;           // Selected tiles
    void add(Tile*);
    void remove(Tile*);
    void clear();
};
```

### Drawing Flow
```
User clicks → MapCanvas::OnMouseDown()
  → Editor::draw(position, alt)
    → Brush::draw(map, tile, param)
      → Tile modifications
        → Action recorded
```

### Bordering
Ground brushes auto-generate borders via `AutoBorder` system in `brush_tables.cpp`.

---

## File Naming Conventions

| Pattern | Meaning |
|---------|---------|
| `*_window.cpp` | Dialog/window class |
| `*_brush.cpp` | Brush implementation |
| `io*.cpp` | I/O handlers |
| `palette_*.cpp` | Palette UI components |
| `live_*.cpp` | Networking components |
| `template*.cpp` | Default map templates |

---

## External Dependencies

| Library | Purpose | Integration |
|---------|---------|-------------|
| wxWidgets | GUI framework | Core UI |
| OpenGL/FreeGLUT | Rendering | Map display |
| Boost.Asio | Networking | Live editing |
| nlohmann_json | JSON parsing | Config |
| libarchive | Archive handling | Asset loading |
| pugixml | XML parsing | Materials/configs |
| zlib | Compression | Map files |

---

## Entry Points

```cpp
// Main application entry
wxIMPLEMENT_APP(Application);

// Application startup
Application::OnInit() {
    g_settings.load();
    ClientVersion::loadVersions();
    g_gui.root = new MainFrame(...);
}

// Map loading
Editor::Editor(copybuffer, filename) {
    map.open(filename);  // → IOMapOTBM::loadMap()
}
```

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
