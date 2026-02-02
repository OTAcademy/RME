# Remere's Map Editor (RME) - Agent Guide

## Project Overview

Remere's Map Editor (RME) is a map editor for game servers derived from the OpenTibia project. It is a fork of the original [RME](https://github.com/hampusborgos/rme) created by Remere, maintained by OTAcademy with significant rendering system upgrades and FPS performance improvements.

**Key Characteristics:**
- **Language**: C++20
- **UI Framework**: wxWidgets 3.2+
- **Graphics**: OpenGL 4.6 with modern shader-based rendering
- **Build System**: CMake 3.28+
- **License**: GPL-3.0

**⚠️ IMPORTANT**: This repository contains many breaking changes and is not stable. It includes:
- OpenGL 4.5+ rendering system
- New library for tooltips
- Better light system
- Fixed minimap lag
- Modern wxWidgets upgrade (WIP)

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
| nanovg | - | 2D vector graphics |
| pugixml | (bundled) | XML parsing |

### External Resources
- **wxMaterialDesignArtProvider**: Fetched via CMake FetchContent for modern UI icons
- **nanovg**: Fallback to bundled version in `ext/nanovg/` if not found via package manager

## Project Structure

```
RME/
├── source/                 # Main source code
│   ├── app/               # Application core (main, settings, preferences, version management)
│   ├── brushes/           # Brush system for map editing
│   │   ├── border/        # Optional border brushes
│   │   ├── carpet/        # Carpet brush with border calculation
│   │   ├── creature/      # Creature spawn brushes
│   │   ├── doodad/        # Decorative item brushes
│   │   ├── door/          # Door brushes
│   │   ├── eraser/        # Eraser tool
│   │   ├── flag/          # Flag/mark brushes
│   │   ├── ground/        # Ground/terrain brushes with auto-border
│   │   ├── house/         # House-related brushes
│   │   ├── managers/      # Brush managers (preview, organization)
│   │   ├── raw/           # Raw item brushes
│   │   ├── spawn/         # Spawn brushes
│   │   ├── table/         # Table brushes
│   │   ├── wall/          # Wall brushes with border calculation
│   │   └── waypoint/      # Waypoint brushes
│   ├── editor/            # Editor core functionality
│   │   ├── managers/      # Editor manager
│   │   ├── operations/    # Editor operations (copy, draw, select)
│   │   └── persistence/   # Save/load editor state
│   ├── game/              # Game world entities
│   │   ├── animation_timer.h
│   │   ├── complexitem.h  # Container items
│   │   ├── creature.h     # NPCs and monsters
│   │   ├── house.h        # House system
│   │   ├── item.h         # Map items
│   │   ├── materials.h    # Material definitions
│   │   ├── spawn.h        # Spawn definitions
│   │   ├── town.h         # Town definitions
│   │   └── waypoints.h    # Waypoint system
│   ├── ingame_preview/    # In-game preview window
│   ├── io/                # File I/O operations
│   │   ├── iomap_otbm.h   # OTBM map format (main)
│   │   └── iomap_otmm.h   # OTMM map format (alternative)
│   ├── live/              # Live collaborative editing
│   │   ├── live_client.h  # Client for live editing
│   │   ├── live_server.h  # Server for live editing
│   │   └── live_socket.h  # Base socket class
│   ├── map/               # Map data structures
│   │   ├── basemap.h      # Base map class
│   │   ├── map.h          # Main map class
│   │   ├── tile.h         # Map tiles
│   │   └── position.h     # 3D position handling
│   ├── net/               # Networking (process communication)
│   ├── palette/           # Brush palette UI
│   │   ├── controls/      # Custom UI controls
│   │   ├── house/         # House palette
│   │   ├── managers/      # Palette manager
│   │   └── panels/        # Palette panels
│   ├── rendering/         # Graphics rendering system
│   │   ├── core/          # Core rendering (GL, textures, shaders)
│   │   ├── drawers/       # Specialized drawers
│   │   │   ├── cursors/   # Cursor rendering
│   │   │   ├── entities/  # Item/creature rendering
│   │   │   ├── overlays/  # Grid, selection overlays
│   │   │   └── tiles/     # Tile rendering
│   │   ├── io/            # Sprite loading, screenshots
│   │   ├── ui/            # UI rendering controllers
│   │   └── utilities/     # FPS counter, lighting
│   ├── ui/                # User interface
│   │   ├── controls/      # Custom controls
│   │   ├── dialogs/       # Dialog windows
│   │   ├── managers/      # UI managers (layout, loading, etc.)
│   │   ├── map/           # Map-related dialogs
│   │   ├── menubar/       # Menu bar handlers
│   │   ├── properties/    # Property windows
│   │   └── toolbar/       # Toolbar system
│   └── util/              # Utilities (common, filesystem, JSON, etc.)
├── data/                  # Client version data
│   ├── clients.xml        # Client version definitions
│   ├── menubar.xml        # Menu bar configuration
│   └── [version]/         # Per-version data (740, 760, 800, etc.)
│       ├── items.otb      # Item database
│       ├── items.xml      # Item definitions
│       ├── grounds.xml    # Ground brush definitions
│       ├── walls.xml      # Wall brush definitions
│       ├── borders.xml    # Border definitions
│       ├── doodads.xml    # Doodad brush definitions
│       ├── creatures.xml  # Creature definitions
│       └── tilesets.xml   # Tileset organization
├── extensions/            # XML extensions for additional brushes/items
├── brushes/               # Brush icon images
├── icons/                 # Application icons
├── tools/                 # Python helper scripts
├── ext/                   # External libraries (nanovg)
└── build/                 # Build output directory
```

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
vcpkg install wxwidgets glad glm asio nlohmann-json fmt libarchive boost-thread nanovg spdlog
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
chmod +x setup_conan.sh
./setup_conan.sh
```

**Build:**
```bash
chmod +x build_linux.sh
./build_linux.sh
```

Output: `build_conan/build/rme`

### macOS

See [Compile on macOS](https://github.com/hjnilsson/rme/wiki/Compiling-on-macOS) wiki page.

### CMake Options

| Option | Description | Default |
|--------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build type (Debug/Release/RelWithDebInfo) | `RelWithDebInfo` |
| `CMAKE_TOOLCHAIN_FILE` | vcpkg toolchain file | - |

## Code Style Guidelines

The project uses **clang-format** for code formatting. Configuration is in `.clang-format`.

**Key Style Rules:**
- **Indentation**: Tabs (width 4)
- **Namespace indentation**: All namespaces indented
- **Braces**: Attach style (K&R), but functions have braces on new line
- **Pointer alignment**: Left (`int* ptr`)
- **Reference alignment**: Left (`int& ref`)
- **Column limit**: 0 (no limit)
- **Preprocessor directives**: Indented before hash
- **Sort includes**: Disabled (preserve order)

**Example:**
```cpp
namespace Game {
	class Item {
	public:
		Item* getParent() {
			return parent;
		}
		
		void setName(const std::string& name);
		
	private:
		Item* parent;
		std::string name;
	};
}
```

**Automatic formatting via GitHub Actions:**
- Pull requests are automatically formatted with clang-format 16
- Commits are added to the PR branch with formatting changes

## Architecture Overview

### Application Flow
1. **Application** (`app/application.cpp`): Main wxApp entry point, initializes GUI
2. **MainFrame** (`app/application.h`): Main window with menu/toolbar
3. **GUI** (`ui/gui.h`): Central GUI manager, singleton pattern
4. **Editor** (`editor/editor.h`): Map editing logic, actions, undo/redo
5. **MapWindow** (`ui/map_window.h`): Displays the map, contains MapCanvas
6. **MapCanvas** (`rendering/ui/map_display.h`): OpenGL canvas for map rendering
7. **MapDrawer** (`rendering/map_drawer.h`): Orchestrates all rendering

### Rendering System
Modern OpenGL 4.6 based renderer with:
- **SpriteBatch**: Efficient sprite rendering using texture arrays
- **ShaderProgram**: GLSL shader management
- **TextureAtlas**: Sprite atlas management
- **LightBuffer**: Light map computation and storage
- **MultiDrawIndirectRenderer**: Efficient batch rendering

Specialized drawers in `rendering/drawers/`:
- `FloorDrawer`: Renders map floors
- `ItemDrawer`: Renders items
- `CreatureDrawer`: Renders creatures with outfit support
- `SelectionDrawer`: Renders selection highlights
- `GridDrawer`: Renders coordinate grid

### Brush System
Brushes are the primary editing tools:
- **Base class**: `brushes/brush.h`
- **GroundBrush**: Terrain with automatic border matching
- **WallBrush**: Walls with automatic corner/border matching
- **DoodadBrush**: Random decorative items
- **CreatureBrush**: Spawn creatures
- **HouseBrush**: Define house areas

### Live Editing
Real-time collaborative editing support:
- **LiveServer**: Host a map editing session
- **LiveClient**: Connect to a remote session
- **LivePeer**: Manages individual client connections
- Uses Boost.ASIO for networking

### Map Format
Primary format is **OTBM** (OpenTibia Binary Map):
- Versions 1-3 supported
- Binary format with zlib compression
- Separate files for houses (.house) and spawns (.spawn)

## Testing

**Note**: The project does not currently have automated unit tests. Testing is done through:

1. **CI Build Verification**: GitHub Actions builds on every PR/push
2. **Manual Testing**: Opening maps, using brushes, testing different client versions
3. **Visual Verification**: Rendering correctness checked manually

### CI Workflows
- `.github/workflows/build.yml`: Linux build with ccache
- `.github/workflows/clang-format.yml`: Automatic code formatting

## Key Files for Common Tasks

### Adding a New Brush Type
1. Create class in `source/brushes/[type]/`
2. Inherit from `Brush` base class
3. Register in `brushes/managers/brush_manager.cpp`
4. Add UI support in palette system

### Adding Client Version Support
1. Add OTB entry in `data/clients.xml`
2. Create data directory `data/[version]/`
3. Add items.otb, items.xml, and brush XML files
4. Update version manager

### Modifying Rendering
1. Core GL: `source/rendering/core/`
2. Drawers: `source/rendering/drawers/`
3. Main orchestrator: `source/rendering/map_drawer.cpp`

### Adding UI Elements
1. Dialogs: `source/ui/dialogs/`
2. Main GUI: `source/ui/gui.cpp`
3. Menu bar: `source/ui/main_menubar.cpp` + `data/menubar.xml`

## Development Conventions

### File Organization
- Headers in same directory as .cpp files
- Use `#pragma once` or include guards
- Forward declarations in `app/rme_forward_declarations.h`

### Naming Conventions
- **Classes**: PascalCase (`MapDrawer`, `GroundBrush`)
- **Methods**: camelCase (`drawMap()`, `getPosition()`)
- **Variables**: camelCase, members with underscore or `m_` prefix
- **Constants**: UPPER_CASE or kPascalCase
- **Filenames**: snake_case.cpp, snake_case.h

### Memory Management
- Use `newd` macro (debug-aware new) for allocations
- Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Raw pointers for non-owning references

### wxWidgets Conventions
- Use `wxString` for UI strings
- Event tables with `DECLARE_EVENT_TABLE()`
- Use `FROM_DIP(widget, size)` for DPI-aware sizing

## Security Considerations

1. **File Loading**: Map files are user-provided; validate all data
2. **Network**: Live editing uses raw TCP; no encryption
3. **Scripting**: Extensions are XML-only, no scripting
4. **Path Traversal**: Ensure file operations stay within data directories

## Troubleshooting

### Common Build Issues

**Windows:**
- Ensure vcpkg is at `C:\vcpkg` or modify `build_windows.bat`
- Use x64 Native Tools Command Prompt for VS2022 if building manually

**Linux:**
- If Conan profile missing: `conan profile detect`
- If wxWidgets not found: `sudo apt install libwxgtk3.2-dev`

### Runtime Issues
- **OpenGL errors**: Ensure OpenGL 4.6+ support
- **Missing sprites**: Check data directory and client version
- **Crash on startup**: Delete config files (location depends on OS)

## Resources

- **Issues**: https://github.com/OTAcademy/rme/issues
- **Discord**: http://discord.gg/OTAcademy
- **Wiki**: https://github.com/hjnilsson/rme/wiki
- **Original Project**: https://github.com/hampusborgos/rme

## Related Projects

- [The Forgotten Server Plus](https://github.com/Zbizu/forgottenserver) - Game server
- [OTClient 1.0](https://github.com/Mehah/otclient) - Game client
