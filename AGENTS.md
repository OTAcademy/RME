# Remere's Map Editor (RME) — Agent Guide

## Project Overview

Remere's Map Editor (RME) is a map editor for game servers derived from the OpenTibia project. It is a fork of the original [RME](https://github.com/hampusborgos/rme) created by Remere, maintained by OTAcademy with significant rendering system upgrades and FPS performance improvements.

**Key Characteristics:**
- **Language**: C++20
- **UI Framework**: wxWidgets 3.2+
- **Graphics**: OpenGL 4.6 with modern shader-based rendering (SpriteBatch, GLSL, texture arrays)
- **2D Vector Graphics**: NanoVG (tooltips, HUD, custom controls)
- **Build System**: CMake 3.28+
- **License**: GPL-3.0

**⚠️ IMPORTANT**: This repository contains many breaking changes and is not stable. It includes:
- OpenGL 4.5+ rendering system (SpriteBatch, MultiDrawIndirect, PostProcessing)
- NanoVG-based tooltips and HUD overlays
- Better light system with per-tile light calculation
- Fixed minimap lag with async renderer
- In-game preview window with walking animation
- Modern wxWidgets upgrade (WIP)
- Advanced replace tool with visual similarity matching

## Technology Stack

### Core Dependencies
| Library | Minimum Version | Purpose |
|---------|-----------------|---------|
| wxWidgets | 3.0+ | Cross-platform GUI framework |
| Boost | 1.55.0+ | Threading, ASIO, utilities |
| OpenGL | 4.6 | Graphics rendering |
| GLAD | 0.1.36 | OpenGL function loading |
| GLM | 1.0.1+ | OpenGL mathematics |
| LibArchive | 3.7.7+ | Compressed file support (OTGZ) |
| spdlog | 1.15.0+ | Logging |
| nlohmann-json | 3.11.3+ | JSON parsing |
| nanovg | - | 2D vector graphics (tooltips, HUD) |
| pugixml | (bundled) | XML parsing |

### External Resources
- **wxMaterialDesignArtProvider**: Fetched via CMake FetchContent for modern UI icons
- **nanovg**: Fallback to bundled version in `ext/nanovg/` if not found via package manager

---

## Project Structure

```
RME/
├── source/                    # Main source code (~600 files)
│   ├── app/                   # Application core
│   │   ├── application.cpp    #   wxApp entry point, startup/shutdown
│   │   ├── client_version.*   #   Client version definitions & loading
│   │   ├── definitions.h      #   Global defines, macros, constants
│   │   ├── extension.*        #   Extension loading
│   │   ├── main.h             #   Precompiled header
│   │   ├── preferences.*      #   Preferences dialog
│   │   ├── rme_forward_declarations.h  # Forward decls
│   │   ├── settings.*         #   Configuration persistence
│   │   ├── updater.*          #   Auto-update checking
│   │   └── managers/
│   │       └── version_manager.*   # Client version management
│   │
│   ├── brushes/               # Brush system for map editing
│   │   ├── brush.h/cpp        #   Base Brush class
│   │   ├── brush_enums.h      #   Brush type enums
│   │   ├── brush_utility.*    #   Shared brush utilities
│   │   ├── border/            #   Border brush computation
│   │   ├── carpet/            #   Carpet brush + border calc (9 files)
│   │   ├── creature/          #   Creature spawn brushes
│   │   ├── doodad/            #   Decorative item brushes (7 files)
│   │   ├── door/              #   Door brushes
│   │   ├── eraser/            #   Eraser tool
│   │   ├── flag/              #   Flag/mark brushes
│   │   ├── ground/            #   Ground/terrain + auto-border (9 files)
│   │   ├── house/             #   House + house exit brushes (4 files)
│   │   ├── raw/               #   Raw item brushes
│   │   ├── spawn/             #   Spawn brushes
│   │   ├── table/             #   Table brushes + adjacency (9 files)
│   │   ├── wall/              #   Wall brushes + border calc (9 files)
│   │   ├── waypoint/          #   Waypoint brushes
│   │   └── managers/
│   │       ├── brush_manager.*              # Global brush registry
│   │       ├── autoborder_preview_manager.* # Real-time border preview
│   │       └── doodad_preview_manager.*     # Doodad preview generation
│   │
│   ├── editor/                # Editor core functionality
│   │   ├── editor.*           #   Central controller: draw/undraw, orchestration
│   │   ├── editor_factory.*   #   Editor creation factory
│   │   ├── editor_tabs.*      #   Tab control for multiple maps
│   │   ├── action.*           #   Undo/redo action system
│   │   ├── action_queue.*     #   Action stack management
│   │   ├── copybuffer.*       #   Copy/paste functionality
│   │   ├── selection.*        #   Selected tiles management
│   │   ├── selection_thread.* #   Async selection processing
│   │   ├── dirty_list.*       #   Change tracking for networking
│   │   ├── hotkey_manager.*   #   Keyboard shortcut management
│   │   ├── map_session.h      #   Map session state
│   │   ├── managers/
│   │   │   └── editor_manager.*  # High-level editor lifecycle
│   │   ├── operations/           # Discrete editor operations
│   │   │   ├── draw_operations.*       # Brush drawing logic
│   │   │   ├── selection_operations.*  # Selection manipulation
│   │   │   ├── copy_operations.*       # Copy/paste operations
│   │   │   ├── clean_operations.h      # Map cleanup operations
│   │   │   ├── search_operations.h     # Search/find operations
│   │   │   └── map_version_changer.*   # Client version migration
│   │   └── persistence/
│   │       ├── editor_persistence.*    # Save/load editor state
│   │       └── tileset_exporter.*      # Tileset export
│   │
│   ├── game/                  # Game world entities & data
│   │   ├── item.*             #   Map items (id, subtype, attributes)
│   │   ├── item_attributes.*  #   Dynamic item attributes
│   │   ├── items.*            #   Item type database (OTB/XML loading)
│   │   ├── creature.*         #   Individual creature instance
│   │   ├── creatures.*        #   Creature type database
│   │   ├── outfit.h           #   Outfit definition (looktype, colors)
│   │   ├── complexitem.*      #   Container, Teleport, Door items
│   │   ├── house.*            #   House system
│   │   ├── spawn.*            #   Spawn definitions
│   │   ├── town.*             #   Town definitions
│   │   ├── waypoints.*        #   Waypoint system
│   │   ├── materials.*        #   Material/tileset loading from XML
│   │   ├── sprites.h          #   Sprite constants & definitions
│   │   ├── animation_timer.*  #   Global animation timer
│   │   └── preview_preferences.* # In-game preview settings
│   │
│   ├── ingame_preview/        # In-game preview window (10 files)
│   │   ├── ingame_preview_window.*     # Top-level preview window
│   │   ├── ingame_preview_canvas.*     # OpenGL canvas for preview
│   │   ├── ingame_preview_renderer.*   # Tile/creature rendering
│   │   ├── ingame_preview_manager.*    # Preview lifecycle management
│   │   └── floor_visibility_calculator.* # Multi-floor visibility
│   │
│   ├── io/                    # File I/O operations
│   │   ├── iomap_otbm.*       #   OTBM binary map format (primary)
│   │   ├── iomap_otmm.*       #   OTMM map format (alternative)
│   │   ├── iomap.*            #   Base map I/O interface
│   │   ├── filehandle.*       #   Binary read/write helpers
│   │   ├── templates.h        #   Default map templates
│   │   └── loaders/
│   │       ├── dat_loader.*   #   Client .dat file loader
│   │       ├── spr_loader.*   #   Client .spr sprite loader
│   │       └── otfi_loader.*  #   OTFI format loader
│   │
│   ├── live/                  # Live collaborative editing (15 files)
│   │   ├── live_server.*      #   Host a live editing session
│   │   ├── live_client.*      #   Connect to remote session
│   │   ├── live_peer.*        #   Individual connection handler
│   │   ├── live_socket.*      #   Socket wrapper (Boost.Asio)
│   │   ├── live_packets.h     #   Network packet definitions
│   │   ├── live_action.*      #   Networked action synchronization
│   │   ├── live_manager.*     #   Live session lifecycle
│   │   └── live_tab.*         #   Live session UI tab
│   │
│   ├── map/                   # Map data structures
│   │   ├── map.*              #   Main Map class (width, height, spawns, houses)
│   │   ├── basemap.*          #   Base map storage, tile access (QuadTree)
│   │   ├── tile.*             #   Tile data: ground, items, creature, spawn
│   │   ├── tile_operations.*  #   Tile manipulation utilities
│   │   ├── position.h         #   3D position (x, y, z) handling
│   │   ├── map_region.*       #   Spatial partitioning for efficient lookup
│   │   ├── spatial_hash_grid.h #  Hash-grid spatial index for viewport queries
│   │   ├── map_allocator.h    #   Memory allocation for map nodes
│   │   ├── map_statistics.*   #   Map analysis & statistics
│   │   ├── map_search.*       #   Map search utilities
│   │   ├── tileset.*          #   Tileset categories & organization
│   │   ├── otml.*             #   OTML format parser
│   │   └── operations/
│   │       └── map_processor.*  # Batch map processing
│   │
│   ├── net/                   # Networking (process communication)
│   │   ├── net_connection.*   #   TCP connection management
│   │   ├── process_com.*      #   Inter-process communication
│   │   └── rme_net.*          #   RME networking protocol
│   │
│   ├── palette/               # Brush palette UI system
│   │   ├── palette_window.*   #   Main palette container
│   │   ├── palette_common.*   #   Shared palette components
│   │   ├── palette_creature.* #   Creature palette
│   │   ├── palette_waypoints.*#   Waypoint palette
│   │   ├── palette_brushlist.*#   Brush selection list
│   │   ├── controls/
│   │   │   ├── brush_button.*        # Clickable brush button
│   │   │   └── virtual_brush_grid.*  # Virtualized brush grid (performance)
│   │   ├── house/
│   │   │   ├── house_palette.*       # House management palette
│   │   │   └── edit_house_dialog.*   # House editing dialog
│   │   ├── managers/
│   │   │   └── palette_manager.*     # Palette lifecycle management
│   │   └── panels/
│   │       ├── brush_palette_panel.* # Brush palette panel
│   │       └── brush_panel.*         # Individual brush panel
│   │
│   ├── rendering/             # Graphics rendering system (~148 files)
│   │   ├── map_drawer.*       #   Top-level render orchestrator
│   │   │
│   │   ├── core/              #   Core rendering infrastructure (52 files)
│   │   │   ├── sprite_batch.*           # Batched sprite rendering (texture arrays)
│   │   │   ├── shader_program.*         # GLSL shader compilation & linking
│   │   │   ├── texture_atlas.*          # Sprite atlas management
│   │   │   ├── texture_array.*          # GL texture array abstraction
│   │   │   ├── texture_garbage_collector.* # Async texture cleanup
│   │   │   ├── atlas_manager.*          # Atlas lifecycle management
│   │   │   ├── multi_draw_indirect_renderer.* # MDI batch rendering
│   │   │   ├── primitive_renderer.*     # Lines, rectangles, triangles
│   │   │   ├── text_renderer.*          # NanoVG text rendering
│   │   │   ├── game_sprite.*            # Game sprite loading & management
│   │   │   ├── editor_sprite.*          # Editor-specific sprites
│   │   │   ├── graphics.*               # Central graphics manager
│   │   │   ├── render_view.*            # Viewport/camera state
│   │   │   ├── drawing_options.*        # Per-frame draw settings
│   │   │   ├── coordinate_mapper.*      # Screen ↔ map coordinate conversion
│   │   │   ├── animator.*               # Sprite animation system
│   │   │   ├── light_buffer.*           # Light map computation & storage
│   │   │   ├── pixel_buffer_object.*    # Async pixel transfers (PBO)
│   │   │   ├── ring_buffer.*            # Triple-buffered streaming
│   │   │   ├── sync_handle.h            # GL fence sync wrapper
│   │   │   ├── shared_geometry.*        # Shared VAO/VBO for quads
│   │   │   ├── gl_resources.h           # RAII GL object wrappers
│   │   │   ├── gl_scoped_state.h        # Scoped GL state (FBO, blend, scissor)
│   │   │   ├── render_timer.*           # Frame timing
│   │   │   ├── sprite_instance.h        # Per-sprite draw data
│   │   │   ├── sprite_light.h           # Per-sprite light data
│   │   │   ├── outfit_colorizer.*       # Outfit color transformation
│   │   │   ├── outfit_colors.*          # Outfit color palette
│   │   │   └── minimap_colors.h         # Minimap color lookup
│   │   │
│   │   ├── drawers/           #   Specialized rendering drawers
│   │   │   ├── map_layer_drawer.*       # Per-layer map rendering
│   │   │   ├── minimap_drawer.*         # Minimap tile rendering
│   │   │   ├── minimap_renderer.*       # Full minimap renderer (async)
│   │   │   ├── tiles/
│   │   │   │   ├── floor_drawer.*       # Floor/level rendering
│   │   │   │   ├── tile_renderer.*      # Individual tile rendering
│   │   │   │   ├── tile_color_calculator.* # Tile tint/color logic
│   │   │   │   └── shade_drawer.*       # Underground shade effect
│   │   │   ├── entities/
│   │   │   │   ├── item_drawer.*        # Item rendering
│   │   │   │   ├── creature_drawer.*    # Creature rendering with outfits
│   │   │   │   ├── creature_name_drawer.* # Creature name labels
│   │   │   │   └── sprite_drawer.*      # Generic sprite rendering
│   │   │   ├── overlays/
│   │   │   │   ├── selection_drawer.*   # Selection highlight rendering
│   │   │   │   ├── grid_drawer.*        # Coordinate grid rendering
│   │   │   │   ├── brush_overlay_drawer.* # Brush preview overlay
│   │   │   │   ├── preview_drawer.*     # Preview ghost rendering
│   │   │   │   └── marker_drawer.*      # Marker/flag rendering
│   │   │   └── cursors/
│   │   │       ├── brush_cursor_drawer.*  # Brush cursor rendering
│   │   │       ├── drag_shadow_drawer.*   # Drag & drop shadow
│   │   │       └── live_cursor_drawer.*   # Live editing cursors
│   │   │
│   │   ├── postprocess/       #   Post-processing pipeline
│   │   │   ├── post_process_manager.*   # Effect chain management
│   │   │   └── effects/
│   │   │       ├── scanline.cpp         # CRT scanline effect
│   │   │       ├── screen.cpp           # Screen-space effects
│   │   │       └── xbrz.cpp            # xBRZ pixel-art upscaler
│   │   │
│   │   ├── shaders/           #   GLSL shader files
│   │   │   ├── sprite_batch.vert       # Vertex shader
│   │   │   └── sprite_batch.frag       # Fragment shader
│   │   │
│   │   ├── io/                #   Sprite/texture loading
│   │   │   ├── game_sprite_loader.*    # Game sprite file loading
│   │   │   ├── editor_sprite_loader.*  # Editor sprite loading
│   │   │   ├── screen_capture.*        # Screen capture utility
│   │   │   └── screenshot_saver.*      # Screenshot file saving
│   │   │
│   │   ├── ui/                #   Map canvas & controllers (28 files)
│   │   │   ├── map_display.*           # **MapCanvas**: main OpenGL canvas widget
│   │   │   ├── drawing_controller.*    # Mouse drawing logic
│   │   │   ├── selection_controller.*  # Selection interaction
│   │   │   ├── navigation_controller.* # Pan/scroll navigation
│   │   │   ├── zoom_controller.*       # Zoom logic
│   │   │   ├── keyboard_handler.*      # Keyboard input handling
│   │   │   ├── clipboard_handler.*     # Clipboard operations
│   │   │   ├── brush_selector.*        # Brush selection UI
│   │   │   ├── map_menu_handler.*      # Right-click context menu
│   │   │   ├── popup_action_handler.*  # Popup menu actions
│   │   │   ├── map_status_updater.*    # Status bar updates
│   │   │   ├── screenshot_controller.* # Screenshot capture controller
│   │   │   ├── tooltip_drawer.*        # NanoVG tooltip rendering
│   │   │   └── minimap_window.*        # Minimap overview window
│   │   │
│   │   └── utilities/         #   Rendering utilities (13 files)
│   │       ├── light_drawer.*          # Light overlay rendering
│   │       ├── light_calculator.*      # Per-tile light computation
│   │       ├── fps_counter.*           # FPS measurement & display
│   │       ├── frame_pacer.*           # Frame timing/pacing
│   │       ├── sprite_icon_generator.* # Sprite → wxBitmap conversion
│   │       ├── tile_describer.*        # Tile content description
│   │       └── wx_utils.h             # wxWidgets rendering helpers
│   │
│   ├── ui/                    # User interface (~183 files)
│   │   ├── gui.*              #   Central GUI controller (singleton)
│   │   ├── gui_ids.h          #   wxWidgets ID constants
│   │   ├── gui_autoborder_ext.* # Autoborder extension UI
│   │   ├── main_frame.*       #   Main application window
│   │   ├── main_menubar.*     #   Menu bar (actions, shortcuts)
│   │   ├── main_toolbar.*     #   Main toolbar
│   │   ├── menubar_loader.*   #   XML → menu bar loading
│   │   ├── map_window.*       #   Map window container (holds MapCanvas)
│   │   ├── map_tab.*          #   Tab for each open map
│   │   ├── map_popup_menu.*   #   Right-click popup menu builder
│   │   ├── map_statistics_dialog.* # Map statistics viewer
│   │   ├── theme.h            #   UI theme definitions
│   │   ├── artprovider.*      #   Custom art/icon provider
│   │   ├── pngfiles.*         #   Embedded PNG image data
│   │   ├── dcbutton.*         #   Custom DC-drawn button control
│   │   ├── numbertextctrl.*   #   Numeric text input control
│   │   ├── positionctrl.*     #   Position (x,y,z) input control
│   │   ├── dialog_helper.*    #   Dialog utility functions
│   │   ├── dialog_util.*      #   Additional dialog helpers
│   │   │
│   │   ├── controls/          #   Custom UI controls
│   │   │   ├── modern_button.*       # Modern styled button
│   │   │   ├── sortable_list_box.*   # Sortable list control
│   │   │   └── item_buttons.h        # Item button definitions
│   │   │
│   │   ├── dialogs/           #   Dialog windows
│   │   │   ├── find_dialog.*         # Find/search dialog
│   │   │   ├── goto_position_dialog.*# Go-to position dialog
│   │   │   ├── outfit_chooser_dialog.*    # Outfit chooser
│   │   │   ├── outfit_preview_panel.*     # Outfit preview
│   │   │   └── outfit_selection_grid.*    # Outfit grid selector
│   │   │
│   │   ├── managers/          #   UI lifecycle managers (16 files)
│   │   │   ├── layout_manager.*      # Window layout save/restore
│   │   │   ├── loading_manager.*     # Loading screen management
│   │   │   ├── gl_context_manager.*  # OpenGL context lifecycle
│   │   │   ├── minimap_manager.*     # Minimap state management
│   │   │   ├── recent_files_manager.*# Recent files list
│   │   │   ├── search_manager.*      # Search state management
│   │   │   ├── status_manager.*      # Status bar management
│   │   │   └── welcome_manager.*     # Welcome screen management
│   │   │
│   │   ├── menubar/           #   Menu bar action handlers (14 files)
│   │   │   ├── menubar_action_manager.* # Action registration
│   │   │   ├── file_menu_handler.*      # File menu actions
│   │   │   ├── map_actions_handler.*    # Map menu actions
│   │   │   ├── navigation_menu_handler.*# Navigation actions
│   │   │   ├── palette_menu_handler.*   # Palette toggle actions
│   │   │   ├── search_handler.*         # Search menu actions
│   │   │   └── view_settings_handler.*  # View settings actions
│   │   │
│   │   ├── toolbar/           #   Toolbar system (18 files)
│   │   │   ├── toolbar_registry.*    # Toolbar registration
│   │   │   ├── toolbar_factory.*     # Toolbar creation factory
│   │   │   ├── toolbar_layout.*      # Toolbar positioning
│   │   │   ├── toolbar_persistence.* # Toolbar state save/load
│   │   │   ├── standard_toolbar.*    # Standard file/edit toolbar
│   │   │   ├── brush_toolbar.*       # Brush selection toolbar
│   │   │   ├── size_toolbar.*        # Brush size toolbar
│   │   │   ├── position_toolbar.*    # Position display toolbar
│   │   │   └── light_toolbar.*       # Light settings toolbar
│   │   │
│   │   ├── map/               #   Map-related dialogs (8 files)
│   │   │   ├── map_properties_window.*   # Map properties editor
│   │   │   ├── import_map_window.*       # Map import dialog
│   │   │   ├── export_tilesets_window.*  # Tileset export dialog
│   │   │   └── towns_window.*            # Town management
│   │   │
│   │   ├── properties/        #   Property editor windows (28 files)
│   │   │   ├── properties_window.*       # Main properties editor
│   │   │   ├── object_properties_base.*  # Base property window
│   │   │   ├── old_properties_window.*   # Legacy properties
│   │   │   ├── attribute_service.*       # Item attribute service
│   │   │   ├── property_validator.*      # Property validation
│   │   │   ├── property_applier.*        # Property application
│   │   │   ├── teleport_service.*        # Teleport property helpers
│   │   │   ├── container_properties_window.*  # Container editor
│   │   │   ├── creature_properties_window.*   # Creature editor
│   │   │   ├── depot_properties_window.*      # Depot editor
│   │   │   ├── podium_properties_window.*     # Podium/outfit display
│   │   │   ├── spawn_properties_window.*      # Spawn editor
│   │   │   ├── splash_properties_window.*     # Splash/fluid editor
│   │   │   └── writable_properties_window.*   # Writable item editor
│   │   │
│   │   ├── replace_tool/      #   Advanced replace tool (21 files)
│   │   │   ├── replace_tool_window.*     # Main replace tool window
│   │   │   ├── replacement_engine.*      # Core replacement logic
│   │   │   ├── rule_manager.*            # Rule CRUD management
│   │   │   ├── rule_builder_panel.*      # Rule creation UI
│   │   │   ├── rule_list_control.*       # Rule list display
│   │   │   ├── rule_card_renderer.*      # Visual rule cards
│   │   │   ├── card_panel.*              # Card layout panel
│   │   │   ├── library_panel.*           # Item library browser
│   │   │   ├── item_grid_panel.*         # Item grid display
│   │   │   ├── visual_similarity_service.* # Visual similarity matching
│   │   │   └── ALGORITHM_DOCS.md         # Algorithm documentation
│   │   │
│   │   ├── about_window.*     # About dialog
│   │   ├── welcome_dialog.*   # Startup welcome screen
│   │   ├── find_item_window.* # Item search dialog
│   │   ├── replace_items_window.* # Legacy bulk replacement
│   │   ├── browse_tile_window.* # Tile contents browser
│   │   ├── result_window.*    # Search results window
│   │   ├── extension_window.* # Extension management
│   │   ├── add_item_window.*  # Add item dialog
│   │   ├── add_tileset_window.* # Add tileset dialog
│   │   ├── tileset_window.*   # Tileset browser
│   │   ├── dat_debug_view.*   # DAT file debug viewer
│   │   ├── live_dialogs.*     # Live editing dialogs
│   │   └── tool_options_*     # Tool options surface/window
│   │
│   └── util/                  # Utilities
│       ├── common.*           #   Common helpers, string utils
│       ├── file_system.*      #   Filesystem utilities
│       ├── json.h             #   JSON helper aliases
│       ├── nanovg_canvas.*    #   NanoVG canvas wrapper for wxWidgets
│       ├── nvg_utils.h        #   NanoVG utility functions
│       ├── virtual_item_grid.*#   Virtualized item grid rendering
│       └── agents.md          #   Codebase encyclopedia (this doc subset)
│
├── data/                      # Client version data
│   ├── clients.xml            #   Client version definitions
│   ├── menubar.xml            #   Menu bar XML configuration
│   └── [version]/             #   Per-version data (740, 760, 800, etc.)
│       ├── items.otb          #     Item database
│       ├── items.xml          #     Item definitions
│       ├── grounds.xml        #     Ground brush definitions
│       ├── walls.xml          #     Wall brush definitions
│       ├── borders.xml        #     Border definitions
│       ├── doodads.xml        #     Doodad brush definitions
│       ├── creatures.xml      #     Creature definitions
│       └── tilesets.xml       #     Tileset organization
├── extensions/                # XML extensions for additional brushes/items
├── brushes/                   # Brush icon images
├── icons/                     # Application icons
├── tools/                     # Python helper scripts
├── ext/                       # External libraries (nanovg, etc.)
└── build/                     # Build output directory
```

---

## Build Instructions

### Windows (vcpkg)

**Prerequisites:**
- Visual Studio 2022 with "Desktop development with C++" workload
- CMake 3.23+
- vcpkg installed at `C:\vcpkg`

**Build:**
```cmd
build_windows.bat
```

Output: `build\Release\rme.exe`

**Manual vcpkg dependency installation:**
```cmd
vcpkg install --triplet x64-windows wxwidgets glad glm asio nlohmann-json fmt libarchive boost-thread nanovg spdlog
```

### Linux (Conan + apt)

**Prerequisites:**
```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-pip ninja-build
sudo apt install -y libgtk-3-dev libgl1-mesa-dev libglu1-mesa-dev libwxgtk3.2-dev
```

**Setup (one-time):**
```bash
chmod +x setup_conan.sh && ./setup_conan.sh
```

**Build:**
```bash
chmod +x build_linux.sh && ./build_linux.sh
```

Output: `build_conan/build/rme`

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build type (Debug/Release/RelWithDebInfo) | `RelWithDebInfo` |
| `CMAKE_TOOLCHAIN_FILE` | vcpkg toolchain file | - |

---

## Architecture Overview

### Layered Architecture

```
┌──────────────────────────────────────────────────────────────────────────┐
│                         Application Layer                                │
│  application.cpp, gui.cpp, main_frame.cpp, main_menubar.cpp              │
│  main_toolbar.cpp, preferences.cpp, settings.cpp                         │
├──────────────────────────────────────────────────────────────────────────┤
│                    UI Layer (wxWidgets)                                   │
│  Palette system | Toolbar system | Property editors | Dialogs            │
│  Replace tool   | Manager classes | Custom controls                      │
├──────────────────────────────────────────────────────────────────────────┤
│                  Rendering Layer (OpenGL 4.6 + NanoVG)                   │
│  MapDrawer orchestrator → SpriteBatch → TextureAtlas → ShaderProgram     │
│  Drawers: tiles, entities, overlays, cursors | PostProcess pipeline      │
│  Controllers: drawing, selection, zoom, navigation, tooltips             │
├────────────────┬───────────────────┬─────────────────────────────────────┤
│  Editor Core   │   Brush System    │    Map Data Layer                    │
│  editor.cpp    │  brush.h + impls  │  map.cpp, basemap.cpp               │
│  action.cpp    │  15 brush types   │  tile.cpp, position.h               │
│  selection.cpp │  managers/preview │  spatial_hash_grid.h                 │
│  operations/*  │                   │  map_region.cpp                      │
├────────────────┴───────────────────┴─────────────────────────────────────┤
│                      Game Data Layer                                     │
│  item.cpp, creature.cpp, house.cpp, spawn.cpp, town.cpp, materials.cpp   │
├──────────────────────────────────────────────────────────────────────────┤
│                         I/O Layer                                        │
│  iomap_otbm.cpp | iomap_otmm.cpp | filehandle.cpp | loaders/*            │
├──────────────────────────────────────────────────────────────────────────┤
│                     Networking Layer                                      │
│  live_server.cpp | live_client.cpp | live_peer.cpp | live_socket.cpp      │
└──────────────────────────────────────────────────────────────────────────┘
```

### Application Flow
1. **Application** (`app/application.cpp`): Main wxApp entry point, initializes GUI
2. **MainFrame** (`ui/main_frame.cpp`): Main window with menu/toolbar
3. **GUI** (`ui/gui.h`): Central GUI manager, singleton pattern
4. **Editor** (`editor/editor.h`): Map editing logic, actions, undo/redo
5. **MapWindow** (`ui/map_window.h`): Container for the map canvas
6. **MapCanvas** (`rendering/ui/map_display.h`): OpenGL canvas for map rendering
7. **MapDrawer** (`rendering/map_drawer.h`): Top-level render orchestrator

### Startup / Load Flow
```
Application::OnInit()
  → g_settings.load()
  → ClientVersion::loadVersions()
  → Materials::loadMaterials()       // XML brush/tileset definitions
  → GUI::CreateMainFrame()
     → MainFrame → MapWindow → MapCanvas (OpenGL context)
     → Graphics::initialize()        // Shaders, SpriteBatch, TextureAtlas
  → Editor::loadMap(filename)
     → IOMapOTBM::loadMap()          // Binary map loading
```

---

## Rendering System (Deep Dive)

The rendering system uses **modern OpenGL 4.6** with a deferred batching architecture.

### Render Pipeline
```
MapDrawer::DrawMap()
  ├── RenderView::update()              // Camera, viewport state
  ├── MapLayerDrawer::drawLayer()       // Per-floor iteration
  │   ├── FloorDrawer::draw()           // Floor tiles
  │   │   └── TileRenderer::render()    // Individual tile
  │   │       ├── ItemDrawer::draw()    // Items on tile
  │   │       ├── CreatureDrawer::draw()// Creatures
  │   │       └── TileColorCalculator   // Tinting
  │   └── ShadeDrawer::draw()           // Underground shade
  ├── Overlay Drawers
  │   ├── SelectionDrawer::draw()       // Selection highlight
  │   ├── GridDrawer::draw()            // Grid lines
  │   ├── BrushOverlayDrawer::draw()    // Brush preview
  │   ├── PreviewDrawer::draw()         // Ghost preview
  │   └── MarkerDrawer::draw()          // Map markers
  ├── Cursor Drawers
  │   ├── BrushCursorDrawer::draw()     // Cursor shape
  │   ├── DragShadowDrawer::draw()      // Drag shadow
  │   └── LiveCursorDrawer::draw()      // Remote cursors
  ├── LightDrawer::draw()               // Light overlay
  ├── PostProcessManager::apply()       // Post-processing effects
  └── TooltipDrawer::draw()             // NanoVG tooltips (HUD)
```

### Key Rendering Classes
| Class | Purpose |
|-------|---------|
| `SpriteBatch` | Batches sprites into texture-array draw calls |
| `ShaderProgram` | Compiles and manages GLSL shaders |
| `TextureAtlas` | Packs sprites into GPU texture arrays |
| `TextureArray` | OpenGL texture array abstraction |
| `MultiDrawIndirectRenderer` | GL_MULTI_DRAW_INDIRECT batching |
| `PrimitiveRenderer` | Lines, rectangles, triangles |
| `RingBuffer` | Triple-buffered GPU streaming |
| `PixelBufferObject` | Async CPU↔GPU pixel transfers |
| `SyncHandle` | GL fence synchronization |
| `SharedGeometry` | Shared full-screen quad VAO/VBO |
| `TextRenderer` | NanoVG text rendering |
| `Animator` | Sprite frame animation |
| `TextureGarbageCollector` | Deferred texture cleanup |

---

## Brush System

Base class: `Brush` in `brushes/brush.h`

```cpp
class Brush {
    virtual void draw(BaseMap* map, Tile* tile, void* parameter) = 0;
    virtual void undraw(BaseMap* map, Tile* tile) = 0;
    virtual std::string getName() const = 0;
    // Type checking: isGround(), isWall(), isDoodad(), etc.
    // Type casting: asGround(), asWall(), asDoodad(), etc.
};
```

### Brush Types (15 total)
| Brush | Directory | Purpose |
|-------|-----------|---------|
| `GroundBrush` | `brushes/ground/` | Terrain tiles with auto-bordering |
| `WallBrush` | `brushes/wall/` | Walls with corner/door logic |
| `DoodadBrush` | `brushes/doodad/` | Decorative objects with randomization |
| `CarpetBrush` | `brushes/carpet/` | Carpets with edge detection |
| `TableBrush` | `brushes/table/` | Tables with adjacency matching |
| `CreatureBrush` | `brushes/creature/` | Place creatures |
| `SpawnBrush` | `brushes/spawn/` | Spawn zones |
| `HouseBrush` | `brushes/house/` | House tile assignment |
| `HouseExitBrush` | `brushes/house/` | House exit markers |
| `DoorBrush` | `brushes/door/` | Door placement |
| `WaypointBrush` | `brushes/waypoint/` | NPC waypoints |
| `FlagBrush` | `brushes/flag/` | PZ/noPVP/etc. zone flags |
| `RAWBrush` | `brushes/raw/` | Single raw item placement |
| `EraserBrush` | `brushes/eraser/` | Remove items |
| `OptionalBorderBrush` | `brushes/border/` | Optional border brushes |

### Brush Managers
| Class | File | Purpose |
|-------|------|---------|
| `BrushManager` | `brushes/managers/brush_manager.*` | Global brush registry, `getBrush(name)` |
| `AutoborderPreviewManager` | `brushes/managers/autoborder_preview_manager.*` | Real-time border preview calculation |
| `DoodadPreviewManager` | `brushes/managers/doodad_preview_manager.*` | Doodad placement preview |

---

## Data Model

### Tile (`map/tile.h`)
```cpp
class Tile {
    TileLocation* location;  // Position on map (x, y, z)
    Item* ground;            // Ground item (unique_ptr)
    ItemVector items;        // Stacked items
    Creature* creature;      // Monster/NPC (one per tile)
    Spawn* spawn;            // Spawn zone reference
    uint32_t house_id;       // House ownership
    // Flags: blocking, PZ, noPVP, etc.
};
```

### Item (`game/item.h`)
```cpp
class Item {
    uint16_t id;             // Item type ID
    uint16_t subtype;        // Count/charges/fluid
    bool selected;           // Selection state
    ItemAttributes* attributes; // Dynamic attributes (text, actionid, etc.)
    // Factory: Item::Create(id, subtype)
};
```

### ItemType (`game/items.h`)
```cpp
class ItemType {
    // Properties loaded from items.otb/dat
    bool blockSolid, blockProjectile, moveable, stackable;
    uint8_t miniMapColor;
    Brush* brush;            // Associated brush if any
    GameSprite* sprite;      // Rendering sprite
};
extern Items g_items;        // Global item database
```

### Map Storage (`map/basemap.h`)
```cpp
class BaseMap {
    // QuadTree-based spatial storage
    // Access: getTile(x, y, z) → Tile*
    // Mutation: setTile(x, y, z, unique_ptr<Tile>) → unique_ptr<Tile>
    // Uses MapAllocator for memory pooling
    // SpatialHashGrid for viewport queries
};
```

---

## Editor Operations

### Undo/Redo System
```cpp
// action.h / action_queue.h
Action → stores tile snapshots (before/after)
BatchAction → groups multiple actions
ActionQueue → undo/redo stack
editor.addAction(action) → pushes to queue
actionQueue->undo() / redo()
```

### Drawing Flow
```
User click → MapCanvas::OnMouseDown()
  → DrawingController::handleMouseDown()
    → Editor::draw(position)
      → DrawOperations::performDraw()
        → Brush::draw(map, tile, param)
          → Tile modifications → Action recorded
```

### Editor Operations (`editor/operations/`)
| File | Purpose |
|------|---------|
| `draw_operations.*` | Brush drawing, undrawing, border recalculation |
| `selection_operations.*` | Select, deselect, move, rotate, flip |
| `copy_operations.*` | Copy, cut, paste with offset |
| `clean_operations.h` | Map cleanup (remove invalid items, etc.) |
| `search_operations.h` | Find items/tiles matching criteria |
| `map_version_changer.*` | Migrate map between client versions |

---

## UI System

### Window Hierarchy
```
MainFrame (wxFrame)
├── MainMenuBar (wxMenuBar)
├── MainToolbar + specialized toolbars (wxAuiToolBar)
├── PaletteWindow (wxPanel) — brush selection
├── MapWindow (wxPanel)
│   └── MapCanvas (wxGLCanvas) — OpenGL rendering
├── MinimapWindow (wxPanel)
├── InGamePreviewWindow (wxFrame) — separate preview
└── Various Dialogs (wxDialog)
```

### Toolbar System (`ui/toolbar/`)
Modular toolbar system with factory pattern:
| Toolbar | Purpose |
|---------|---------|
| `StandardToolbar` | File, edit, undo/redo actions |
| `BrushToolbar` | Brush type selection |
| `SizeToolbar` | Brush size controls |
| `PositionToolbar` | Current position display |
| `LightToolbar` | Light settings |

### Replace Tool System (`ui/replace_tool/`)
Advanced item replacement with:
- **Rule-based replacement**: Create rules mapping source → target items
- **Visual similarity matching**: Find visually similar items automatically
- **Card-based UI**: Modern card layout for rule display
- **Library panel**: Browse all items with grid display

### Property Editors (`ui/properties/`)
Type-specific property windows with shared base class:
- Container, Creature, Depot, Podium, Spawn, Splash, Writable, Teleport
- Shared services: `AttributeService`, `PropertyValidator`, `PropertyApplier`

### Menu Bar Handlers (`ui/menubar/`)
Decomposed into domain-specific handlers:
- File, Map, Navigation, Palette, Search, View Settings

---

## In-Game Preview System

Separate OpenGL window showing the map as it appears in-game:
| File | Purpose |
|------|---------|
| `ingame_preview_window.*` | Top-level preview window with controls |
| `ingame_preview_canvas.*` | OpenGL canvas with player movement |
| `ingame_preview_renderer.*` | Game-accurate tile/creature rendering |
| `ingame_preview_manager.*` | Preview lifecycle management |
| `floor_visibility_calculator.*` | Multi-floor visibility calculation |

---

## I/O System

### Map Formats
| Format | Files | Description |
|--------|-------|-------------|
| OTBM | `io/iomap_otbm.*` | Binary map format (primary, zlib compressed) |
| OTMM | `io/iomap_otmm.*` | Alternative map format |

### Client Data Loaders (`io/loaders/`)
| Loader | Purpose |
|--------|---------|
| `dat_loader.*` | Client .dat file (item/creature appearances) |
| `spr_loader.*` | Client .spr file (sprite pixel data) |
| `otfi_loader.*` | OTFI format metadata |

---

## Networking (Live Editing)

Real-time collaborative map editing via TCP (Boost.ASIO):
| File | Purpose |
|------|---------|
| `live_server.*` | Host a live editing session |
| `live_client.*` | Connect to remote session |
| `live_peer.*` | Individual connection handler |
| `live_socket.*` | Socket wrapper |
| `live_packets.h` | Packet type definitions |
| `live_action.*` | Action synchronization over network |
| `live_manager.*` | Session lifecycle management |
| `live_tab.*` | Chat/collaboration UI tab |

---

## Utility Systems

### NanoVG Integration (`util/`)
| File | Purpose |
|------|---------|
| `nanovg_canvas.*` | wxWidgets ↔ NanoVG bridge, sprite texture caching |
| `nvg_utils.h` | Color conversion, drawing helpers |
| `virtual_item_grid.*` | Virtualized scrollable item grid with NanoVG |

### Map Utilities (`map/`)
| File | Purpose |
|------|---------|
| `spatial_hash_grid.h` | Hash-based spatial index for fast viewport queries |
| `map_region.*` | Region-based spatial partitioning |
| `otml.*` | OTML (OT Markup Language) parser |
| `map_statistics.*` | Map content analysis |
| `map_search.*` | Map search utilities |
| `tile_operations.*` | Tile manipulation helpers |
| `map_processor.*` | Batch tile processing |

---

## Global Singletons

| Variable | Type | Purpose |
|----------|------|---------|
| `g_gui` | `GUI` | Central GUI controller |
| `g_brushes` | `Brushes` (via `BrushManager`) | Brush registry |
| `g_items` | `Items` | Item type database |
| `g_creatures` | `Creatures` | Creature database |
| `g_materials` | `Materials` | Materials/tilesets |
| `g_settings` | `Settings` | Configuration persistence |
| `g_graphics` | `Graphics` (via `GraphicsManager`) | Sprite/texture management |

---

## Code Style Guidelines

The project uses **clang-format** for code formatting. Configuration is in `.clang-format`.

**Key Style Rules:**
- **Indentation**: Tabs (width 4)
- **Namespace indentation**: All namespaces indented
- **Braces**: Attach style (K&R), functions have braces on new line
- **Pointer alignment**: Left (`int* ptr`)
- **Column limit**: 0 (no limit)
- **Sort includes**: Disabled (preserve order)
- **C++ Standard**: C++20 (`std::format`, `std::ranges`, structured bindings, concepts)

**Naming Conventions:**
- **Classes**: PascalCase (`MapDrawer`, `GroundBrush`)
- **Methods**: camelCase (`drawMap()`, `getPosition()`)
- **Variables**: camelCase, members with underscore suffix or `m_` prefix
- **Constants**: UPPER_CASE or kPascalCase
- **Filenames**: snake_case.cpp, snake_case.h

**Memory Management:**
- Prefer `std::unique_ptr` for owned objects
- Raw pointers for non-owning references
- `MapAllocator` for tile/floor node pooling

**wxWidgets Conventions:**
- Use `wxString` for UI strings
- Use `FROM_DIP(widget, size)` macro for DPI-aware sizing
- Event tables with `DECLARE_EVENT_TABLE()` or `Bind()`

**Automatic formatting via GitHub Actions:**
- Pull requests are automatically formatted with clang-format 16
- Commits are added to the PR branch with formatting changes

---

## Key Files for Common Tasks

### Adding a New Brush Type
1. Create directory `source/brushes/[type]/`
2. Inherit from `Brush` base class (`brushes/brush.h`)
3. Register in `brushes/managers/brush_manager.cpp`
4. Add palette support in `palette/` system
5. Add rendering support in `rendering/drawers/` if needed

### Adding Client Version Support
1. Add OTB entry in `data/clients.xml`
2. Create data directory `data/[version]/`
3. Add items.otb, items.xml, and brush XML files
4. Update `app/managers/version_manager.cpp`

### Modifying Rendering
1. Core GL infrastructure: `source/rendering/core/`
2. Specialized drawers: `source/rendering/drawers/`
3. Top-level orchestrator: `source/rendering/map_drawer.cpp`
4. Post-processing: `source/rendering/postprocess/`
5. Shaders: `source/rendering/shaders/`

### Adding UI Elements
1. Dialogs: `source/ui/dialogs/`
2. Property windows: `source/ui/properties/`
3. Menu actions: `source/ui/menubar/` + `data/menubar.xml`
4. Toolbars: `source/ui/toolbar/`
5. Custom controls: `source/ui/controls/`
6. Manager classes: `source/ui/managers/`

### Adding a Property Editor
1. Create window in `source/ui/properties/`
2. Inherit from `ObjectPropertiesBase`
3. Use `AttributeService` for attribute access
4. Use `PropertyValidator` for validation
5. Register in `PropertiesWindow`

---

## Testing

**Note**: The project has limited automated tests. A test file exists at `source/test_autoborder_preview.cpp`.

Testing is primarily done through:
1. **CI Build Verification**: GitHub Actions builds on every PR/push
2. **Manual Testing**: Opening maps, using brushes, testing different client versions
3. **Visual Verification**: Rendering correctness checked manually

### CI Workflows (`.github/workflows/`)
- Build verification (Linux with ccache)
- Automatic clang-format formatting

---

## Security Considerations

1. **File Loading**: Map files are user-provided; validate all data
2. **Network**: Live editing uses raw TCP; no encryption
3. **Scripting**: Extensions are XML-only, no scripting engine
4. **Path Traversal**: Ensure file operations stay within data directories

---

## Troubleshooting

### Common Build Issues

**Windows:**
- Ensure vcpkg is at `C:\vcpkg` or modify `build_windows.bat`
- Use x64 Native Tools Command Prompt for VS2022 if building manually

**Linux:**
- If Conan profile missing: `conan profile detect`
- If wxWidgets not found: `sudo apt install libwxgtk3.2-dev`

### Runtime Issues
- **OpenGL errors**: Ensure OpenGL 4.6+ support (check with GPU-Z or glxinfo)
- **Missing sprites**: Check data directory and client version
- **Crash on startup**: Delete config files (location depends on OS)

---

## Resources

- **Issues**: https://github.com/OTAcademy/rme/issues
- **Discord**: http://discord.gg/OTAcademy
- **Wiki**: https://github.com/hjnilsson/rme/wiki
- **Original Project**: https://github.com/hampusborgos/rme

## Related Projects

- [The Forgotten Server Plus](https://github.com/Zbizu/forgottenserver) - Game server
- [OTClient 1.0](https://github.com/Mehah/otclient) - Game client
