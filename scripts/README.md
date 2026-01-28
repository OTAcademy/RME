# Lua Scripting Documentation for Remere's Map Editor

This guide provides a comprehensive overview of the Lua scripting capabilities within Remere's Map Editor (RME). It covers the file structure, API reference, and examples to help you create tools, generators, and extensions.

---

## Table of Contents

1.  [Getting Started](#getting-started)
2.  [Script Structure](#script-structure)
3.  [API Reference](#api-reference)
    *   [App (Global)](#app-global)
    *   [Global Variables](#global-variables)
    *   [Map](#map)
    *   [Tile](#tile)
    *   [Item](#item)
    *   [Creature & Spawn](#creature--spawn)
    *   [Selection](#selection)
    *   [Brushes](#brushes)
    *   [Image](#image)
    *   [UI / Dialogs](#ui--dialogs)
4.  [Examples](#examples)

---

## Getting Started

Scripts allow you to automate tasks, create custom tools, and extend the editor's functionality.

### Location
All Lua scripts must be placed in the `scripts` directory of your RME installation.
The editor scans this directory on startup. Files must have the `.lua` extension.

### Hello World
Create a file `scripts/hello.lua`:

```lua
-- Simple script that shows an alert
app.alert("Hello from RME Lua!")
```

---

## Script Structure

Scripts are executed sequentially when the editor loads. You can define global functions or use the API immediately.


### Script Metadata

There are two ways to create scripts in RME:

#### Option 1: Single File Script (The Easy Way)
This is best for simple scripts, tools, or quick experiments. You just create a single `.lua` file in the `scripts/` directory.

You can define metadata in the file header using comments. The editor parses these to display information in the menus.

```lua
-- My Script Name v1.0
-- A description of what this script does.
-- Additional description lines...

-- Code starts here
```

The first line is treated as the script name (and optional version).
Subsequent lines starting with `--` are treated as the description.
Metadata parsing stops at the first empty line or code line.

You can also use special tags for specific fields:
*   `@Tile: Script Name` - Sets the display name.
*   `@Description: Text` - Sets the description.
*   `@Author: Name` - Sets the script author.
*   `@Version: 1.0` - Sets the script version.
*   `@Shortcut: KeyCombination` - Assigns a default keyboard shortcut (e.g., `Ctrl+Shift+K`).

```lua
-- @Title: My Script v1.0
-- @Description: A description of what this script does.
-- @Author: John Doe
-- @Version: 1.0
-- @Shortcut: Ctrl+Shift+G
```

#### Option 2: Package Script (The Advanced Way)

This method is recommended for complex tools or extensions that require multiple files, custom modules, or external resources (like images or data files).

1.  Create a subdirectory in `scripts/` (e.g., `scripts/my_cool_tool/`).
2.  Create a `manifest.lua` file inside that directory.
3.  Place your main script and resources in that directory.

**manifest.lua Example:**
```lua
return {
    name = "My Cool Tool",
    description = "A tool that does amazing things.",
    author = "Jane Doe",
    version = "1.0",
    main = "main.lua",        -- Entry point script (defaults to main.lua)
    autorun = true,           -- Run automatically on startup (optional)
    shortcut = "Ctrl+Alt+T"   -- Keyboard shortcut (optional)
}
```

When using packages, the `SCRIPT_DIR` global variable in your main script will point to your package directory, allowing you to easily load resources:
```lua
local img = Image.fromFile(SCRIPT_DIR .. "/assets/icon.png")
```

### Best Practices
*   Wrap your functionality in local functions.
*   Use `app.transaction` for any operations that modify the map to ensure Undo/Redo support works correctly.
*   Use `app.alert` for simple user feedback.

---

## API Reference

### App (Global)

The `app` table provides access to global editor state and utility functions.

| Property/Function | Type | Description |
| :--- | :--- | :--- |
| `app.version` | string | The current RME version. |
| `app.apiVersion` | number | The API version number (currently 2). |
| `app.map` | [Map](#map) | Returns the currently active Map object (or nil). |
| `app.selection` | [Selection](#selection) | Returns the current Selection object. |
| `app.keyboard` | [Keyboard](#keyboard) | State of modifier keys. |
| `app.borders` | table | Table of all available borders. |
| `app.brush` | [Brush](#brushes) | The currently selected brush. |
| `app.brushSize` | number | Current brush size (radius). |
| `app.brushShape` | string | Current brush shape ("circle" or "square"). |
| `app.spawnTime` | number | Default spawn time for creatures. |
| `app.getDataDirectory()` | function | Returns the absolute path to the data directory. |
| `app.hasMap()` | function | Returns `true` if a map is currently open. |
| `app.refresh()` | function | Refreshes the map view. |
| `app.copy()` | function | Copies current selection to internal clipboard. |
| `app.cut()` | function | Cuts current selection to internal clipboard. |
| `app.paste()` | function | Pastes from internal clipboard to current map position. |
| `app.setClipboard(text)` | function | Sets the system clipboard text. |
| `app.setCameraPosition(x, y, z)` | function | Moves the camera to the specified map coordinates. |
| `app.storage(name)` | function | Returns a per-script storage helper (`load/save/clear`) backed by JSON. |
| `app.mapView` | table | Access to map overlay APIs (`addOverlay/removeOverlay/setEnabled/registerShow`). |
| `app.events` | [Events](#events) | Access to the event system. |
| `app.yield()` | function | Yields to process pending UI events. Use in long-running loops to prevent UI freeze. |
| `app.sleep(ms)` | function | Sleeps for the given milliseconds (max 10000). Blocks the UI thread. |

#### Keyboard
Access via `app.keyboard`.

| Method | Description |
| :--- | :--- |
| `isCtrlDown()` | Returns true if Control key is pressed. |
| `isShiftDown()` | Returns true if Shift key is pressed. |
| `isAltDown()` | Returns true if Alt key is pressed. |

#### Events
The `app.events` object allows scripts to listen for global editor events.

```lua
-- Register an event listener
local listenerId = app.events:on("spawnChange", function(action, tile)
    if action == "add" then
        print("Spawn added at " .. tile.x .. "," .. tile.y)
    end
end)

-- Remove an event listener
app.events:off(listenerId)
```

**Available Events:**

| Event Name | Arguments | Description |
| :--- | :--- | :--- |
| `spawnChange` | `action` ("add"\|"remove"), `tile` | Triggered when a spawn is added to or removed from a tile. |
| `mapLoad` | - | Triggered when a new map is loaded. |
| `mapSave` | `filename` | Triggered when the current map is saved. |
| `selectionChange` | - | Triggered when the tile selection changes. |
| `brushChange` | `brushName` | Triggered when the active brush changes. |
| `floorChange` | `newFloor`, `oldFloor` | Triggered when the visible map floor changes. |
```lua
-- Simple
app.alert("Message")

-- Advanced
app.alert({
    title = "Confirmation",
    text = "Are you sure?",
    buttons = {"Yes", "No"}
})
```

#### Transaction (Undo/Redo)
All map modifications **must** be wrapped in a transaction.

```lua
app.transaction("My Script Action", function()
    -- Modify map here
    local tile = app.map:getTile(100, 100, 7)
    if tile then
        tile:addItem(2160) -- Add Crystal Coin
    end
end)
```

---

### Global Variables

These variables are automatically set by the engine before your script runs.

| Variable | Type | Description |
| :--- | :--- | :--- |
| `SCRIPT_DIR` | string | The directory containing the currently executing script. Use this to load resources (images, data files) relative to your script location. **Note:** This is only available when running scripts from a file. |

**Usage:**
```lua
-- Load an image from the script's directory
local myImage = Image.fromFile(SCRIPT_DIR .. "/my_image.png")

-- Load a data file
local file = io.open(SCRIPT_DIR .. "/config.txt", "r")
```

---

### Map

The `Map` object represents the open map file.

| Property/Method | Description |
| :--- | :--- |
| `name` | Name of the map. |
| `width`, `height` | Dimensions of the map. |
| `tileCount` | Total number of tiles. |
| `getTile(x, y, z)` | Returns a [Tile](#tile) or `nil`. |
| `getTile(position)` | Same as above, using a position table/object. |
| `getOrCreateTile(x, y, z)` | Returns a Tile, creating it if it doesn't exist. |
| `tiles` | Iterator for looping through all tiles. |

**Usage:**
```lua
for tile in app.map.tiles do
    -- Process tile
end
```

---

### Tile

A `Tile` represents a specific coordinate (x, y, z) on the map.

| Property/Method | Description |
| :--- | :--- |
| `position` | Returns `{x, y, z}` table. |
| `x`, `y`, `z` | Coordinate properties. |
| `ground` | The ground [Item](#item) (read/write). |
| `items` | Lua table of all items on the tile (excluding ground). |
| `itemCount` | Number of items on the tile. |
| `creature` | The [Creature](#creature--spawn) on the tile (or nil). |
| `spawn` | The [Spawn](#creature--spawn) on the tile (or nil). |
| `houseId` | ID of the house this tile belongs to. |
| `isPZ`, `isBlocking` | Flags (Protection Zone, etc.). |
| `addItem(id, [count])` | Adds an item by ID. Returns the new [Item](#item). |
| `removeItem(item)` | Removes a specific item object. |
| `setCreature(name)` | Sets the creature. |
| `removeCreature()` | Removes the creature. |
| `setSpawn(radius)` | Sets a spawn zone. |
| `removeSpawn()` | Removes the spawn zone. |
| `getTopItem()` | Returns the item visually on top. |

---

### Item

Represents an item on a tile or in a container.

| Property/Method | Description |
| :--- | :--- |
| `id` | The item ID (read-only). |
| `name` | The item name. |
| `count` | Stack count (or subtype). |
| `actionId`, `uniqueId` | Properties for scripting. |
| `text`, `description` | Text properties (e.g., for signs). |
| `isStackable`, `isMoveable` | Type checks. |
| `clone()` | Returns a copy of the item. |
| `rotate()` | Rotates the item if possible. |

---

### Creature & Spawn

**Creature**
*   `name`: Creature name.
*   `spawnTime`: Time in seconds.
*   `direction`: Enum (`Direction.NORTH`, `Direction.EAST`, etc.).
*   `isNpc`: Boolean.

**Spawn**
*   `radius` (or `size`): The spawn radius/size (1-50).

---

### Selection

Access via `app.selection`.

| Property/Method | Description |
| :--- | :--- |
| `tiles` | Table of selected [Tile](#tile) objects. |
| `size` | Number of selected tiles. |
| `bounds` | Table `{min={x,y,z}, max={x,y,z}}`. |
| `clear()` | Deselects everything. |
| `add(tile, [item])` | Adds a tile (and optional item) to selection. |
| `remove(tile, [item])` | Removes from selection. |

---

### Brushes

Scripts can interact with the current brush.

*   `app.setBrush(name)`: Switches the active brush.
*   `Brushes.get(name)`: Returns a Brush object.
*   `Brushes.getNames()`: Returns a list of all brush names.

**Brush Object:**
*   `name`, `id`, `type` (e.g., "terrain", "doodad").

---

### Image

The `Image` class allows you to load and manipulate images from files or game sprites.

#### Creating Images

```lua
-- From file
local img = Image.fromFile("path/to/image.png")
local img = Image("path/to/image.png")
local img = Image{path = "path/to/image.png"}

-- From item sprite (by item ID)
local img = Image.fromItemSprite(2160)  -- Crystal Coin
local img = Image{itemid = 2160}

-- From raw sprite ID
local img = Image.fromSprite(100)
local img = Image{spriteid = 100}

-- Using SCRIPT_DIR for relative paths
local img = Image.fromFile(SCRIPT_DIR .. "/my_image.png")
```

#### Properties

| Property | Type | Description |
| :--- | :--- | :--- |
| `width` | number | Width of the image in pixels. |
| `height` | number | Height of the image in pixels. |
| `valid` | boolean | `true` if the image was loaded successfully. |
| `path` | string | File path (if loaded from file). |
| `spriteId` | number | Sprite ID (if loaded from sprite). |
| `isFromSprite` | boolean | `true` if the image was loaded from a game sprite. |

#### Methods

| Method | Description |
| :--- | :--- |
| `resize(width, height, [smooth])` | Returns a new resized image. `smooth` defaults to `true`. Set to `false` for pixel-perfect scaling. |
| `scale(factor, [smooth])` | Returns a new scaled image by the given factor. |

**Scaling Modes:**
*   `smooth = true` (default): Uses bilinear interpolation. Good for photos, may look blurry on pixel art.
*   `smooth = false`: Uses nearest-neighbor interpolation. Keeps pixels sharp, ideal for sprites.

```lua
-- Smooth scaling (may look blurry)
local large = img:resize(64, 64, true)

-- Pixel-perfect scaling (sharp pixels)
local large = img:resize(64, 64, false)
local doubled = img:scale(2.0, false)
```

---

### UI / Dialogs

You can create custom dialogs using the `Dialog` class.

**Constructor:**
`Dialog(title)` or `Dialog({title="...", resizable=true, dockable=true, onclose=function...})`

*   `onclose`: Optional callback function executed when the dialog is closed.

**Widgets (Chainable):**
*   `label({text="My Label"})`
*   `input({id="my_input", text="Default", label="Prompt"})`
*   `number({id="num", value=10, min=0, max=100})`
*   `check({id="cb", text="Enable Feature", selected=true})`
*   `button({text="Click Me", onclick=function() ... end})`
*   `combobox({id="combo", options={"A", "B"}, option="A"})`
*   `color({id="col", label="Pick Color"})`
*   `item({id="itm", itemid=100})`
*   `image({id="img", itemid=100})` or `image({path="...", smooth=false})`
*   `file({id="f", filename="test.txt", save=false})`
*   `mapCanvas({id="preview"})`

**Layout:**
*   `box({orient="horizontal"})` / `endbox()`
*   `wrap()` / `endwrap()`

**Showing:**
*   `show()`: Displays the dialog modally. The script waits here until the dialog is closed.
*   `close()`: Closes the dialog programmatically.
*   `show({wait=true, center="parent"|"screen", center=false, bounds=...})`: `center` defaults to parent when no explicit `x/y` or `bounds` are set.

**Dialog Properties:**
*   `dlg.activeTab`: The current tab label (string) or `nil` if no tab is active.

**List/Grid Options (Common)**
*   `onchange`, `ondoubleclick`, `onleftclick`, `onrightclick`, `oncontextmenu`.
*   `list` supports `icon_size`, `item_height`, `show_text`, `smooth`, and per-item `tooltip`/`image`/`icon`.
*   `grid` supports `item_size`, `cell_size`, `label_wrap`, `show_text`, and per-item `tooltip`/`image`.

### Widget Reference

All widget methods return the `Dialog` object itself, allowing for method chaining. Most widgets accept an `options` table.

#### **Layout & Grouping**
| Widget | Options | Description |
| :--- | :--- | :--- |
| `box(options)` | `{orient="vertical"\|"horizontal", label="Title"}` | Starts a container box. If `label` is provided, draws a border with a title. |
| `endbox()` | - | Ends the current box. |
| `wrap()` | - | Starts a horizontal wrapper. Elements will be placed side-by-side. |
| `endwrap()` | - | Ends the wrapper. |
| `newrow()` | - | Forces the next widget to start on a new line (in default layout). |
| `separator()` | - | Draws a horizontal line separator. |
| `tab(options)` | `{id="tab_id", text="Tab Name", button=false, index=1, onclick=func, oncontextmenu=func}` | Starts a new tab page or a tab-like button (`button=true`). |
| `endtabs()` | - | Ends the tab definition. |
| `panel(options)` | `{bgcolor=Color.white, padding=5, margin=5, expand=true, height=30}` | Starts a styled container panel. Supports background color, padding, margin, and standard layout options. |
| `endpanel()` | - | Ends the current panel. |

#### **Common Widget Options**
Most widgets support these additional layout and styling properties:

| Property | Type | Description |
| :--- | :--- | :--- |
| `expand` | boolean | If `true`, the widget expands to fill available space in the layout direction. |
| `align` | string | Horizontal alignment: `"left"`, `"center"`, `"right"`. |
| `valign` | string | Vertical alignment: `"top"`, `"center"`, `"bottom"`. |
| `width`, `height` | number | Explicit width/height in pixels. |
| `min_width`, `min_height` | number | Minimum dimensions. |
| `max_width`, `max_height` | number | Maximum dimensions. |
| `margin` | number | Outer margin (all sides). |
| `padding` | number | Inner padding (for containers like `panel` or `box`). |
| `bgcolor` | string/Color | Background color (e.g., `Color.red` or `"#FF0000"`). |
| `fgcolor` | string/Color | Foreground (text) color. |
| `font_size` | number | Font size in points. |
| `font_weight` | string | Font weight: `"normal"`, `"bold"`. |


#### **Input Widgets**
| Widget | Options | Description |
| :--- | :--- | :--- |
| `label(options)` | `{text="Text"}` | Displays static text. |
| `input(options)` | `{id="key", label="Label", text="Default", onchange=func}` | Single-line text input. |
| `number(options)` | `{id="key", label="Label", value=0, min=0, max=100, decimals=0}` | Numeric input spinner. |
| `slider(options)` | `{id="key", label="Label", value=0, min=0, max=100}` | Horizontal slider bar. |
| `check(options)` | `{id="key", text="Label", selected=false, onclick=func}` | Checkbox (boolean). |
| `radio(options)` | `{id="key", text="Label", selected=false}` | Radio button (boolean). |
| `combobox(options)` | `{id="key", label="Label", options={"A", "B"}, option="A"}` | Dropdown selection list. |
| `color(options)` | `{id="key", label="Label", color={red=255, green=0, blue=0}}` | Color picker button. Returns `{red,green,blue}` table. |
| `file(options)` | `{id="key", label="Label", filename="file.txt", save=false}` | File picker. `save=true` for save dialog. |
| `item(options)` | `{id="key", label="Label", itemid=100}` | Button showing an item sprite. Click to change item. |
| `image(options)` | `{id="key", label="Label", image=Image, path="...", itemid=100, spriteid=100, width=32, height=32, smooth=true}` | Displays an image. Can load from file, item sprite, or raw sprite. Use `smooth=false` for pixel-perfect scaling. |
| `mapCanvas(options)` | `{id="key", label="Label"}` | Renders a live preview of the current map view. |

#### **Action Widgets**
| Widget | Options | Description |
| :--- | :--- | :--- |
| `button(options)` | `{id="btn_id", text="Click Me", onclick=func, focus=false}` | Standard button. `onclick` is a callback function receiving `(dlg)`. |
| `list(options)` | `{id="lst", items={{text="A", image=Image, tooltip="..."}, {text="B", icon=123}}, oncontextmenu=func}` | List box with tooltips and images (`image` or `icon`). Supports `onchange`, `ondoubleclick`, `onleftclick`, `onrightclick`, `oncontextmenu`. |
| `grid(options)` | `{id="grid", items={{text="", image=Image, tooltip="..."}}, item_size=32, cell_size=32}` | Icon grid with tooltips, per-item images, and size controls. Supports `onchange`, `ondoubleclick`, `onleftclick`, `onrightclick`, `oncontextmenu`. |

#### **Data Access**
*   `dlg.data`: Table of current values `{id = value}`. Can be read or written to.
*   `dlg.values`: Alias for `data`.

#### **Dynamic Updates**
*   `dlg:modify({ widget_id = { property = new_value } })`
    *   Example: `dlg:modify({ my_label = { text = "New Text" } })`
    *   Grid size updates: you can pass `icon_size`/`item_size`/`item_width`/`item_height` and `cell_size`/`cell_width`/`cell_height`. To apply size changes, resend `items` so the image list can be rebuilt.
*   `dlg:repaint()`: Forces a UI refresh.

**Context Menus (Tabs/List/Grid)**
Callbacks can return a menu table:
```lua
oncontextmenu = function(dlg, info)
    return {
        { text = "Action", onclick = function() end },
        { separator = true },
        { text = "Disabled", enabled = false }
    }
end
```

### Map View Overlays

Use `app.mapView` to draw overlays on the map and react to hover changes. Overlays run without requiring a dialog and can still expose UI if desired.

```lua
app.mapView:addOverlay("Light Strength", {
    ondraw = function(ctx)
        -- ctx.view: {x1,y1,x2,y2,z,zoom}
        -- ctx:rect{ x,y,z,w,h,color={r,g,b,a}, filled=true }
    end,
    onhover = function(info)
        -- info.pos, info.screen, info.tile, info.topItem
        return {
            tooltip = { text = "Light: 6", color = {255, 200, 50} },
            highlight = { color = {255, 200, 50, 120}, filled = false, width = 1 }
        }
    end
})
```

To add a toggle in the **Show** menu, register a show input (custom entries are appended after a separator):

```lua
app.mapView:registerShow("Light Strength", "Light Strength", {
    enabled = true,
    ontoggle = function(enabled)
        -- Persist or react to state change
    end
})
```

### Script Storage

```lua
local store = app.storage("settings")
local data = store:load() or {}
data.enabled = true
store:save(data)
```

### HTTP

The `http` table provides HTTP request functionality for scripts.

#### Basic Requests

| Function | Description |
| :--- | :--- |
| `http.get(url, [headers])` | Performs a GET request. Returns response table. |
| `http.post(url, body, [headers])` | Performs a POST request with string body. |
| `http.postJson(url, table, [headers])` | Performs a POST request with JSON body. |

**Response Table:**
```lua
{
    ok = true,           -- true if status is 2xx
    status = 200,        -- HTTP status code
    body = "...",        -- Response body as string
    error = "",          -- Error message if failed
    headers = {}         -- Response headers table
}
```

**Example:**
```lua
local result = http.get("https://api.example.com/data")
if result.ok then
    local data = json.decode(result.body)
    app.alert("Got: " .. data.message)
else
    app.alert("Error: " .. result.error)
end
```

#### Streaming Requests

For large responses or real-time data (like AI APIs), use streaming:

| Function | Description |
| :--- | :--- |
| `http.postStream(url, body, headers)` | Starts streaming POST request. Returns `{sessionId, ok}`. |
| `http.postJsonStream(url, table, headers)` | Starts streaming POST with JSON body. |
| `http.streamRead(sessionId)` | Reads available data from stream. |
| `http.streamStatus(sessionId)` | Checks stream status without reading. |
| `http.streamClose(sessionId)` | Closes and cleans up stream session. |

**streamRead Response:**
```lua
{
    data = "...",        -- New data received since last read
    finished = false,    -- true when stream is complete
    hasError = false,    -- true if an error occurred
    error = "",          -- Error message if hasError
    ok = true,           -- false if error
    status = 200,        -- HTTP status (only when finished)
    headers = {}         -- Response headers (only when finished)
}
```

**Streaming Example:**
```lua
local stream = http.postJsonStream("https://api.openai.com/v1/chat/completions", {
    model = "gpt-4",
    messages = {{ role = "user", content = "Hello" }},
    stream = true
}, {
    ["Authorization"] = "Bearer " .. apiKey
})

if stream.ok then
    local content = ""
    while true do
        local result = http.streamRead(stream.sessionId)
        if result.data ~= "" then
            content = content .. result.data
        end
        if result.finished or result.hasError then
            break
        end
        app.yield() -- Prevent UI freeze
    end
    http.streamClose(stream.sessionId)
end
```

---

## Examples

### 1. Mass Item Replacer
Replaces all items of ID 100 with ID 200 in the current selection.

```lua
local function replaceItems()
    local sel = app.selection
    if sel.isEmpty then
        app.alert("Select an area first!")
        return
    end

    app.transaction("Replace Items", function()
        for _, tile in pairs(sel.tiles) do
            for _, item in pairs(tile.items) do
                if item.id == 100 then
                    -- Create new item
                    tile:addItem(200, item.count)
                    -- Remove old
                    tile:removeItem(item)
                end
            end
        end
    end)
    app.alert("Done!")
end

-- Currently, to run this, you can assign it to a menu or run via console if available.
-- Or simply executing the file runs main scope.
replaceItems()
```

### 2. Terrain Generator UI
A simple UI to generate grass.

```lua
local dlg = Dialog("Grass Generator")

dlg:label({text="Settings"})
dlg:box({orient="horizontal"})
    dlg:number({id="density", label="Density %", value=50, min=0, max=100})
    dlg:check({id="flowers", text="Add Flowers", selected=true})
dlg:endbox()

dlg:button({text="Generate", onclick=function()
    local density = dlg.values.density -- Access values
    local flowers = dlg.values.flowers

    app.transaction("Generate Grass", function()
        -- Logic to place grass...
        app.alert("Generated with density: " .. density)
    end)
end})

dlg:show()
```

*(Note: `dlg.values` is the table where widget values are stored by ID)*

### 3. Image Display Demo

Display images from files and game sprites with different scaling modes.

```lua
-- Load an image from the script's directory
local myImage = Image.fromFile(SCRIPT_DIR .. "/logo.png")

-- Load item sprites
local goldCoin = Image.fromItemSprite(2148)
local crystalCoin = Image{itemid = 2160}

local dlg = Dialog{title = "Image Demo", width = 400}

-- Display image from file
if myImage.valid then
    dlg:label{text = "Logo (" .. myImage.width .. "x" .. myImage.height .. "):"}
    dlg:newrow()
    dlg:image{image = myImage, width = 128, height = 128, smooth = false}
    dlg:newrow()
end

-- Display item sprites with pixel-perfect scaling
dlg:label{text = "Item sprites (64x64, pixel-perfect):"}
dlg:newrow()
dlg:image{itemid = 2148, width = 64, height = 64, smooth = false}
dlg:image{itemid = 2152, width = 64, height = 64, smooth = false}
dlg:image{itemid = 2160, width = 64, height = 64, smooth = false}
dlg:newrow()

-- Dynamic image update
local currentItem = 2148
dlg:image{id = "preview", itemid = currentItem, width = 64, height = 64, smooth = false}
dlg:button{text = "Next Item", onclick = function()
    local items = {2148, 2152, 2160}
    for i, v in ipairs(items) do
        if v == currentItem then
            currentItem = items[(i % #items) + 1]
            break
        end
    end
    dlg:modify{preview = {itemid = currentItem, width = 64, height = 64, smooth = false}}
end}

dlg:button{id = "close", text = "Close"}
dlg:show()
```

---

## Procedural Generation APIs

RME includes powerful procedural generation APIs for creating terrain, caves, dungeons, and more.

### Noise

The `noise` table provides various noise functions for procedural terrain generation.

#### Basic Noise Functions

| Function | Description |
| :--- | :--- |
| `noise.perlin(x, y, seed?, frequency?)` | 2D Perlin noise, returns [-1, 1] |
| `noise.perlin3d(x, y, z, seed?, frequency?)` | 3D Perlin noise |
| `noise.simplex(x, y, seed?, frequency?)` | OpenSimplex2 noise (recommended) |
| `noise.simplex3d(x, y, z, seed?, frequency?)` | 3D OpenSimplex2 noise |
| `noise.simplexSmooth(x, y, seed?, frequency?)` | Smoother OpenSimplex2S variant |
| `noise.cellular(x, y, seed?, frequency?, distanceFunc?, returnType?)` | Cellular/Voronoi noise |
| `noise.value(x, y, seed?, frequency?)` | Value noise (blocky) |
| `noise.valueCubic(x, y, seed?, frequency?)` | Cubic interpolated value noise |

#### Fractal Noise

| Function | Description |
| :--- | :--- |
| `noise.fbm(x, y, seed?, options?)` | Fractional Brownian Motion - layered noise |
| `noise.fbm3d(x, y, z, seed?, options?)` | 3D FBM |
| `noise.ridged(x, y, seed?, options?)` | Ridged fractal noise (mountains, veins) |

**FBM Options:**
```lua
{
    frequency = 0.01,      -- Base frequency
    octaves = 4,           -- Number of noise layers
    lacunarity = 2.0,      -- Frequency multiplier per octave
    gain = 0.5,            -- Amplitude multiplier per octave
    noiseType = "simplex"  -- "perlin", "simplex", "value", "cellular"
}
```

#### Domain Warping

```lua
-- Distort coordinates for organic effects
local warped = noise.warp(x, y, seed, {
    amplitude = 30,
    frequency = 0.01,
    type = "simplex"  -- "simplex", "simplexReduced", "basic"
})
-- warped.x, warped.y contain the distorted coordinates
```

#### Utility Functions

| Function | Description |
| :--- | :--- |
| `noise.normalize(value, min?, max?)` | Normalize [-1,1] to [min, max] (default [0, 1]) |
| `noise.threshold(value, threshold)` | Returns true if value >= threshold |
| `noise.map(value, inMin, inMax, outMin, outMax)` | Map value between ranges |
| `noise.clamp(value, min, max)` | Clamp value to range |
| `noise.lerp(a, b, t)` | Linear interpolation |
| `noise.smoothstep(edge0, edge1, x)` | Smooth interpolation |

#### Batch Generation

```lua
-- Generate noise grid (faster than individual calls)
local grid = noise.generateGrid(0, 0, 100, 100, {
    seed = 1337,
    frequency = 0.02,
    noiseType = "simplex",
    fractal = "fbm",
    octaves = 4
})
-- grid[y][x] contains noise values
```

**Example - Island Generator:**
```lua
local seed = os.time()
local sel = app.selection.bounds
local minX, minY, z = sel.min.x, sel.min.y, sel.min.z
local maxX, maxY = sel.max.x, sel.max.y

app.transaction("Generate Island", function()
    for y = minY, maxY do
        for x = minX, maxX do
            local n = noise.fbm(x, y, seed, {frequency = 0.03, octaves = 4})

            -- Radial falloff for island shape
            local cx, cy = (maxX - minX) / 2, (maxY - minY) / 2
            local dx, dy = (x - minX) - cx, (y - minY) - cy
            local dist = math.sqrt(dx*dx + dy*dy) / math.min(cx, cy)
            n = n * (1 - dist * dist)

            local tile = app.map:getOrCreateTile(x, y, z)
            if n < -0.2 then
                tile:addItem(4608)  -- Water
            elseif n < 0.3 then
                tile:addItem(4526)  -- Grass
            else
                tile:addItem(919)   -- Mountain
            end
        end
    end
end)
```

---

### Algo (Algorithms)

The `algo` table provides procedural generation algorithms.

#### Cellular Automata

```lua
-- Run cellular automata on existing grid
local result = algo.cellularAutomata(grid, {
    iterations = 4,
    birthLimit = 4,   -- Become wall if neighbors >= this
    deathLimit = 3,   -- Stay wall if neighbors >= this
    width = 100,
    height = 100
})

-- Generate a cave map directly
local caveMap = algo.generateCave(100, 100, {
    fillProbability = 0.45,
    iterations = 5,
    birthLimit = 4,
    deathLimit = 3,
    seed = os.time()
})
-- caveMap[y][x] == 1 means wall, 0 means floor
```

#### Erosion

```lua
-- Hydraulic erosion for realistic terrain
local eroded = algo.erode(heightmap, {
    iterations = 50000,
    erosionRadius = 3,
    inertia = 0.05,
    sedimentCapacity = 4.0,
    erosionSpeed = 0.3,
    depositSpeed = 0.3,
    evaporateSpeed = 0.01,
    gravity = 4.0,
    seed = os.time()
})

-- Thermal erosion (talus/slope erosion)
local thermal = algo.thermalErode(heightmap, {
    iterations = 50,
    talusAngle = 0.5,
    erosionAmount = 0.5
})
```

#### Voronoi Diagram

```lua
-- Generate random seed points
local points = algo.generateRandomPoints(100, 100, 20, seed)

-- Generate Voronoi regions
local voronoi = algo.voronoi(100, 100, points)
-- voronoi[y][x] contains region index (1-based)
```

#### Dungeon Generation (BSP)

```lua
local result = algo.generateDungeon(100, 100, {
    minRoomSize = 5,
    maxRoomSize = 15,
    maxDepth = 4,
    seed = os.time()
})

-- result.grid[y][x] == 1 means wall, 0 means floor
-- result.rooms is array of {x, y, width, height}
```

#### Maze Generation

```lua
local maze = algo.generateMaze(51, 51, {seed = os.time()})
-- maze[y][x] == 1 means wall, 0 means passage
```

#### Smoothing

```lua
local smoothed = algo.smooth(grid, {
    iterations = 2,
    kernelSize = 3
})
```

---

### Geo (Geometry)

The `geo` table provides geometric algorithms for drawing shapes and paths.

#### Line Drawing (Bresenham)

```lua
-- 2D line
local points = geo.bresenhamLine(x1, y1, x2, y2)
-- points = {{x=.., y=..}, {x=.., y=..}, ...}

-- 3D line
local points3d = geo.bresenhamLine3d(x1, y1, z1, x2, y2, z2)
```

#### Bezier Curves

```lua
-- Bezier curve through control points
local controlPoints = {
    {x = 0, y = 0},
    {x = 50, y = 100},   -- Control point
    {x = 150, y = 100},  -- Control point
    {x = 200, y = 0}
}
local curve = geo.bezierCurve(controlPoints, 50)  -- 50 steps

-- 3D Bezier
local curve3d = geo.bezierCurve3d(controlPoints3d, 50)
```

#### Flood Fill

```lua
-- Fill region with new value
local filled = geo.floodFill(grid, startX, startY, newValue, {
    eightConnected = false  -- true for 8-directional
})

-- Get positions without modifying grid
local positions = geo.getFloodFillPositions(grid, startX, startY, {
    eightConnected = false
})
```

#### Shapes

```lua
-- Circle (outline or filled)
local circle = geo.circle(centerX, centerY, radius, {filled = true})

-- Ellipse
local ellipse = geo.ellipse(centerX, centerY, radiusX, radiusY, {filled = false})

-- Rectangle
local rect = geo.rectangle(x1, y1, x2, y2, {filled = true})

-- Polygon (outline)
local polygon = geo.polygon({{x=0,y=0}, {x=100,y=0}, {x=50,y=100}})
```

#### Distance Functions

```lua
geo.distance(x1, y1, x2, y2)          -- Euclidean
geo.distanceSq(x1, y1, x2, y2)        -- Squared (faster)
geo.distanceManhattan(x1, y1, x2, y2) -- Manhattan
geo.distanceChebyshev(x1, y1, x2, y2) -- Chebyshev (king's move)
```

#### Point-in-Shape Tests

```lua
geo.pointInCircle(px, py, cx, cy, radius)
geo.pointInRectangle(px, py, x1, y1, x2, y2)
geo.pointInPolygon(px, py, vertices)
```

#### Random Point Generation

```lua
-- Simple random scatter
local points = geo.randomScatter(x1, y1, x2, y2, count, {
    seed = os.time(),
    minDistance = 5  -- Minimum spacing
})

-- Poisson disk sampling (blue noise - evenly distributed)
local blueNoise = geo.poissonDiskSampling(x1, y1, x2, y2, minDistance, {
    seed = os.time(),
    maxAttempts = 30
})
```

**Example - Draw River with Bezier:**
```lua
local sel = app.selection.bounds
local minX, minY, z = sel.min.x, sel.min.y, sel.min.z
local maxX, maxY = sel.max.x, sel.max.y

-- Create curved river path
local controlPoints = {
    {x = minX, y = (minY + maxY) / 2},
    {x = minX + 30, y = minY + 20},
    {x = maxX - 30, y = maxY - 20},
    {x = maxX, y = (minY + maxY) / 2}
}

local riverPath = geo.bezierCurve(controlPoints, 100)

app.transaction("Draw River", function()
    for _, point in ipairs(riverPath) do
        -- Draw circle at each point for river width
        local circle = geo.circle(point.x, point.y, 2, {filled = true})
        for _, cp in ipairs(circle) do
            local tile = app.map:getOrCreateTile(cp.x, cp.y, z)
            if tile then
                tile:addItem(4608)  -- Water
            end
        end
    end
end)
```
