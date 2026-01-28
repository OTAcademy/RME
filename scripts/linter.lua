--[[
//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
]]

---@class Position
---@field x number
---@field y number
---@field z number

---@class Item
---@field id number
---@field clientId number
---@field name string
---@field fullName string
---@field count number
---@field actionId number
---@field uniqueId number
---@field description string
---@field isStackable boolean
---@field isMoveable boolean
---@field isBlocking boolean
---@field isGroundTile boolean
---@field isBorder boolean
---@field isWall boolean
---@field isDoor boolean
---@field hasElevation boolean
---@field zOrder number
local ItemClass = {}

---@class Creature
---@field name string
---@field outfit number|table
---@field direction number
---@field spawnTime number

---@class Spawn
---@field radius number

---@class Tile
---@field x number
---@field y number
---@field z number
---@field items Item[]
---@field itemCount number
---@field ground Item|nil
---@field creature Creature|nil
---@field spawn Spawn|nil
---@field houseId number
---@field isPZ boolean
---@field isBlocking boolean
---@field hasCreature boolean
---@field isHouseTile boolean
---@field hasBorders boolean
---@field hasWall boolean
---@field hasTable boolean
---@field hasCarpet boolean
---@field groundZOrder number
---@field hasSpawn boolean
---@field isNoPvp boolean
---@field isNoLogout boolean
---@field isPvpZone boolean
local TileClass = {}

--- Adds an item to the tile.
---@param id number Item ID.
---@param count? number Stack count or subtype (optional).
---@return Item
function TileClass:addItem(id, count) return {} --[[@as Item]] end

--- Removes a specific item object from the tile.
---@param item Item
function TileClass:removeItem(item) end

--- Moves an item within the tile stack or to another tile.
---@overload fun(self: Tile, fromIdx: integer, toIdx: integer)
---@overload fun(self: Tile, item: Item, toIdx: integer)
---@overload fun(self: Tile, item: Item, destTile: Tile, toIdx?: integer)
function TileClass:moveItem(fromIdxOrItem, toIdxOrDestTile, toIdx) end

--- Sets the creature on the tile.
---@param name string
---@param spawnTime? number
---@param direction? number
function TileClass:setCreature(name, spawnTime, direction) end

--- Removes the creature from the tile.
function TileClass:removeCreature() end

--- Sets the spawn on the tile.
---@param radius number
function TileClass:setSpawn(radius) end

--- Removes the spawn from the tile.
function TileClass:removeSpawn() end

--- Applies wall auto-alignment to the tile.
function TileClass:wallize() end

---@return Item
function TileClass:getTopItem() return {} --[[@as Item]] end

---@return Position
function TileClass:getPosition() return {x=0,y=0,z=0} end

---@class Selection
---@field size number
---@field isEmpty boolean
---@field tiles Tile[]
---@field bounds table<string, Position>
---@field minPosition Position
---@field maxPosition Position
local SelectionClass = {}
--- Starts a batched selection operation (improves performance).
function SelectionClass:start() end
--- Finishes a batched selection operation and commits changes.
function SelectionClass:finish() end

---@class Map
---@field name string
---@field width number
---@field height number
---@field tileCount number
local MapClass = {}
---@param x number
---@param y number
---@param z number
---@return Tile|nil
function MapClass:getTile(x, y, z) return nil end

---@class App
---@field version string
---@field map Map|nil
---@field selection Selection
---@field editor Editor
---@field events Events
---@field mapView MapView
---@field keyboard Keyboard
---@field alert fun(message: string)
---@field refresh fun()
---@field copy fun() Copies current selection to clipboard
---@field cut fun() Cuts current selection to clipboard
---@field paste fun() Pastes from clipboard
---@field transaction fun(name: string, callback: fun())
---@field addContextMenu fun(label: string, callback: fun())
---@field selectRaw fun(id: number)
---@field setCameraPosition fun(x: number, y: number, z: number)
---@field storage fun(name: string): ScriptStorage
---@field yield fun() Yields to process pending UI events (prevents freeze during long operations)
---@field sleep fun(milliseconds: number) Sleeps for the given milliseconds (max 10000). Blocks UI.
app = {}

---@class Keyboard
---@field isCtrlDown fun(): boolean
---@field isShiftDown fun(): boolean
---@field isAltDown fun(): boolean
local Keyboard = {}

---@class Events
---@field on fun(self: Events, eventName: string, callback: fun()): number
---@field off fun(self: Events, listenerId: number): boolean
local Events = {}

---@class Editor
---@field historyIndex number
---@field historySize number
---@field undo fun(self: Editor)
---@field redo fun(self: Editor)
---@field canUndo fun(self: Editor): boolean
---@field canRedo fun(self: Editor): boolean
---@field getHistory fun(self: Editor): table
---@field goToHistory fun(self: Editor, index: number)
local EditorClass = {}

---@alias WidgetOptions {id?: string, align?: "left"|"center"|"right", valign?: "top"|"center"|"bottom", expand?: boolean, width?: number, height?: number, min_width?: number, min_height?: number, max_width?: number, max_height?: number, margin?: number, padding?: number, bgcolor?: string|table, fgcolor?: string|table, font_size?: number, font_weight?: "normal"|"bold"}

---@class Dialog
---@field data? table<string, any>
---@field values? table<string, any>
---@field bounds? table<string, any>
---@field dockable? boolean
---@field activeTab? string|nil
---@field onclose? fun()
---@overload fun(title_or_config: string|{title?: string, resizable?: boolean, dockable?: boolean, id?: string, x?: number, y?: number, width?: number, height?: number, onclose?: fun()}): Dialog
local DialogClass = {}

---@param options {text: string}
---@return Dialog
function DialogClass:label(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, text?: string, onchange?: fun(val: string)}
---@return Dialog
function DialogClass:entry(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, value?: number, min?: number, max?: number, decimals?: number}
---@return Dialog
function DialogClass:number(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, value?: number, min?: number, max?: number}
---@return Dialog
function DialogClass:slider(options) return {} --[[@as Dialog]] end

---@param options {id: string, text: string, selected?: boolean, onclick?: fun(val: boolean)}
---@return Dialog
function DialogClass:check(options) return {} --[[@as Dialog]] end

---@param options {id: string, text: string, selected?: boolean}
---@return Dialog
function DialogClass:radio(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, options: string[], option?: string}
---@return Dialog
function DialogClass:combobox(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, color?: {red: number, green: number, blue: number}}
---@return Dialog
function DialogClass:color(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, filename?: string, save?: boolean}
---@return Dialog
function DialogClass:file(options) return {} --[[@as Dialog]] end

---@param options {id?: string, label?: string, image?: Image, path?: string, itemid?: number, spriteid?: number, width?: number, height?: number, smooth?: boolean}
---@return Dialog
function DialogClass:image(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string, itemid: number}
---@return Dialog
function DialogClass:item(options) return {} --[[@as Dialog]] end

---@param options {id: string, label?: string}
---@return Dialog
function DialogClass:mapCanvas(options) return {} --[[@as Dialog]] end

---@param options {id?: string, width?: number, height?: number, items: {text?: string, icon?: number, image?: Image|string, tooltip?: string}[], selection?: number, icon_size?: number, icon_width?: number, icon_height?: number, item_height?: number, show_text?: boolean, smooth?: boolean, onchange?: fun(dlg: Dialog), ondoubleclick?: fun(dlg: Dialog), oncontextmenu?: fun(dlg: Dialog, info: table), onleftclick?: fun(dlg: Dialog, info: table), onrightclick?: fun(dlg: Dialog, info: table)}
---@return Dialog
function DialogClass:list(options) return {} --[[@as Dialog]] end

---@param options {id?: string, label?: string, items?: {text?: string, tooltip?: string, image?: Image|string}[], width?: number, height?: number, selection?: number, icon_size?: number, icon_width?: number, icon_height?: number, item_size?: number, item_width?: number, item_height?: number, cell_size?: number, cell_width?: number, cell_height?: number, show_text?: boolean, label_wrap?: boolean, ondoubleclick?: fun(dlg: Dialog), onchange?: fun(dlg: Dialog), oncontextmenu?: fun(dlg: Dialog, info: table), onleftclick?: fun(dlg: Dialog, info: table), onrightclick?: fun(dlg: Dialog, info: table)}
---@return Dialog
function DialogClass:grid(options) return {} --[[@as Dialog]] end

---@param options {orient: "vertical"|"horizontal", label?: string}
---@return Dialog
function DialogClass:box(options) return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:endbox() return {} --[[@as Dialog]] end

---@param options? WidgetOptions|{label?: string}
---@return Dialog
function DialogClass:panel(options) return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:endpanel() return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:wrap() return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:endwrap() return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:newrow() return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:separator() return {} --[[@as Dialog]] end

---@param options {id?: string, text: string, button?: boolean, index?: number, onclick?: fun(dlg: Dialog, info: table), oncontextmenu?: fun(dlg: Dialog, info: table)}
---@return Dialog
function DialogClass:tab(options) return {} --[[@as Dialog]] end

---@return Dialog
function DialogClass:endtabs() return {} --[[@as Dialog]] end

---@param options {id?: string, text: string, onclick: fun(), focus?: boolean}
---@return Dialog
function DialogClass:button(options) return {} --[[@as Dialog]] end

---@param options? {wait?: boolean, bounds?: table, center?: boolean|string, center_on?: string}
---@return Dialog
function DialogClass:show(options) return {} --[[@as Dialog]] end

---@param options table<string, any>
--- Grid size updates via modify: pass icon/item/cell size keys and resend `items` to rebuild icons.
---@return Dialog
function DialogClass:modify(options) return {} --[[@as Dialog]] end

function DialogClass:close() end
function DialogClass:clear() end
function DialogClass:layout() end

---@class ScriptStorage
---@field path string
---@field load fun(self: ScriptStorage): table|nil
---@field save fun(self: ScriptStorage, data: table|string): boolean
---@field clear fun(self: ScriptStorage): boolean

---@class MapView
---@field addOverlay fun(self: MapView, id: string, options: MapOverlayOptions): boolean
---@field removeOverlay fun(self: MapView, id: string): boolean
---@field setEnabled fun(self: MapView, id: string, enabled: boolean): boolean
---@field registerShow fun(self: MapView, label: string, id: string, options?: MapOverlayShowOptions): boolean

---@class MapOverlayOptions
---@field enabled? boolean
---@field order? number
---@field ondraw? fun(ctx: MapOverlayContext)
---@field onhover? fun(info: MapOverlayHoverInfo): string|table|nil

---@class MapOverlayContext
---@field view {x1: number, y1: number, x2: number, y2: number, z: number, zoom: number}
---@field rect fun(self: MapOverlayContext, opts: MapOverlayRectOptions)
---@field line fun(self: MapOverlayContext, opts: MapOverlayLineOptions)
---@field text fun(self: MapOverlayContext, opts: MapOverlayTextOptions)

---@class MapOverlayRectOptions
---@field x number
---@field y number
---@field z? number
---@field w? number
---@field h? number
---@field color? {r?: number, g?: number, b?: number, a?: number}
---@field filled? boolean
---@field width? number
---@field screen? boolean
---@field style? "solid"|"dotted"|"dashed"
---@field dashed? boolean

---@class MapOverlayLineOptions
---@field x1 number
---@field y1 number
---@field z1? number
---@field x2 number
---@field y2 number
---@field z2? number
---@field color? {r?: number, g?: number, b?: number, a?: number}
---@field width? number
---@field screen? boolean
---@field style? "solid"|"dotted"|"dashed"
---@field dashed? boolean

---@class MapOverlayTextOptions
---@field x number
---@field y number
---@field z? number
---@field text string
---@field color? {r?: number, g?: number, b?: number, a?: number}
---@field screen? boolean

---@class MapOverlayHoverInfo
---@field pos Position
---@field screen {x: number, y: number}
---@field tile Tile|nil
---@field topItem Item|nil

---@class MapOverlayShowOptions
---@field enabled? boolean
---@field ontoggle? fun(enabled: boolean)

---@class Image
---@field width number
---@field height number
---@field valid boolean
---@field path string
---@field spriteId number
---@field isFromSprite boolean
---@overload fun(): Image
---@overload fun(path: string): Image
---@overload fun(options: {path?: string, itemid?: number, spriteid?: number}): Image
local ImageClass = {}

--- Creates an Image from a file path
---@param path string
---@return Image
function ImageClass.fromFile(path) return {} --[[@as Image]] end

--- Creates an Image from an item sprite ID
---@param itemId number
---@return Image
function ImageClass.fromItemSprite(itemId) return {} --[[@as Image]] end

--- Creates an Image from a raw sprite ID
---@param spriteId number
---@return Image
function ImageClass.fromSprite(spriteId) return {} --[[@as Image]] end


---@param width number
---@param height number
---@param smooth? boolean Use smooth scaling (default true). Set to false for pixel-perfect scaling.
---@return Image
function ImageClass:resize(width, height, smooth) return {} --[[@as Image]] end

--- Scales the image by the specified factor
---@param factor number
---@param smooth? boolean Use smooth scaling (default true). Set to false for pixel-perfect scaling.
---@return Image
function ImageClass:scale(factor, smooth) return {} --[[@as Image]] end

---@class ImageStatics
---@field fromFile fun(path: string): Image
---@field fromItemSprite fun(itemId: number): Image
---@field fromSprite fun(spriteId: number): Image
---@overload fun(path_or_options?: string|{path?: string, itemid?: number, spriteid?: number}): Image

---@type ImageStatics
---@diagnostic disable-next-line: missing-fields
Image = {} --[[@as ImageStatics]]

---@class Color
---@field red number
---@field green number
---@field blue number
---@field alpha number

---@class ColorHelper
---@field red string|table
---@field green string|table
---@field blue string|table
---@field white string|table
---@field black string|table
---@field gray string|table
---@field darkGray string|table
---@field lightGray string|table
---@field orange string|table
---@field yellow string|table
---@field cyan string|table
---@field magenta string|table
---@field transparent string|table
---@field lighten fun(color: string|table|Color, amount: number): Color
---@field darken fun(color: string|table|Color, amount: number): Color

---@type ColorHelper
---@diagnostic disable-next-line: missing-fields
Color = {} --[[@as ColorHelper]]

-- Global variables set by the engine
---@type string The directory containing the currently executing script. Use this to load resources relative to your script.
---@type string The directory containing the currently executing script. Use this to load resources relative to your script.
SCRIPT_DIR = ""

---@class Items
---@field getInfo fun(id: number): table|nil Returns item info table by ID
---@field exists fun(id: number): boolean Check if item ID exists
---@field getMaxId fun(): number Get maximum item ID
---@field findByName fun(name: string, maxResults?: number): table[] Find items by name (case-insensitive contains search)
---@field findIdByName fun(name: string): number|nil Find first item ID matching name (exact first, then contains)
---@field getName fun(id: number): string Get item name by ID
Items = {}

-- Enums
NORTH = 0
EAST = 1
SOUTH = 2
WEST = 3
SOUTHWEST = 4
SOUTHEAST = 5
NORTHWEST = 6
NORTHEAST = 7

-- ============================================
-- PROCEDURAL GENERATION APIS
-- ============================================

---@class Noise
---@field perlin fun(x: number, y: number, seed?: number, frequency?: number): number Perlin noise 2D [-1, 1]
---@field perlin3d fun(x: number, y: number, z: number, seed?: number, frequency?: number): number Perlin noise 3D [-1, 1]
---@field simplex fun(x: number, y: number, seed?: number, frequency?: number): number OpenSimplex2 noise 2D [-1, 1]
---@field simplex3d fun(x: number, y: number, z: number, seed?: number, frequency?: number): number OpenSimplex2 noise 3D [-1, 1]
---@field simplexSmooth fun(x: number, y: number, seed?: number, frequency?: number): number OpenSimplex2S (smoother) [-1, 1]
---@field cellular fun(x: number, y: number, seed?: number, frequency?: number, distanceFunc?: string, returnType?: string): number Cellular/Voronoi noise
---@field cellular3d fun(x: number, y: number, z: number, seed?: number, frequency?: number): number Cellular noise 3D
---@field value fun(x: number, y: number, seed?: number, frequency?: number): number Value noise 2D [-1, 1]
---@field valueCubic fun(x: number, y: number, seed?: number, frequency?: number): number Cubic value noise 2D [-1, 1]
---@field fbm fun(x: number, y: number, seed?: number, options?: NoiseOptions): number Fractional Brownian Motion
---@field fbm3d fun(x: number, y: number, z: number, seed?: number, options?: NoiseOptions): number FBM 3D
---@field ridged fun(x: number, y: number, seed?: number, options?: NoiseOptions): number Ridged fractal noise
---@field warp fun(x: number, y: number, seed?: number, options?: WarpOptions): table Domain warping returns {x, y}
---@field normalize fun(value: number, min?: number, max?: number): number Normalize [-1,1] to [min, max]
---@field threshold fun(value: number, threshold: number): boolean Returns true if value >= threshold
---@field map fun(value: number, inMin: number, inMax: number, outMin: number, outMax: number): number Map value between ranges
---@field clamp fun(value: number, min: number, max: number): number Clamp value to range
---@field lerp fun(a: number, b: number, t: number): number Linear interpolation
---@field smoothstep fun(edge0: number, edge1: number, x: number): number Smooth interpolation
---@field clearCache fun() Clear noise generator cache
---@field generateGrid fun(x1: number, y1: number, x2: number, y2: number, options?: GridOptions): table[][] Generate noise grid
noise = {}

---@class NoiseOptions
---@field frequency? number Base frequency (default 0.01)
---@field octaves? number Number of layers (default 4)
---@field lacunarity? number Frequency multiplier per octave (default 2.0)
---@field gain? number Amplitude multiplier per octave (default 0.5)
---@field noiseType? string "perlin" | "simplex" | "value" | "cellular"

---@class WarpOptions
---@field amplitude? number Warp amplitude (default 30)
---@field frequency? number Warp frequency (default 0.01)
---@field type? string "simplex" | "simplexReduced" | "basic"

---@class GridOptions
---@field seed? number Random seed
---@field frequency? number Noise frequency
---@field noiseType? string "perlin" | "simplex" | "cellular" | "value"
---@field fractal? string "none" | "fbm" | "ridged"
---@field octaves? number Fractal octaves
---@field lacunarity? number Fractal lacunarity
---@field gain? number Fractal gain

---@class Algo
---@field cellularAutomata fun(grid: table[][], options?: CellularAutomataOptions): table[][] Run cellular automata
---@field generateCave fun(width: number, height: number, options?: CaveOptions): table[][] Generate cave map
---@field erode fun(heightmap: table[][], options?: ErosionOptions): table[][] Hydraulic erosion
---@field thermalErode fun(heightmap: table[][], options?: ThermalErosionOptions): table[][] Thermal erosion
---@field smooth fun(grid: table[][], options?: SmoothOptions): table[][] Gaussian-like smoothing
---@field voronoi fun(width: number, height: number, points: table[]): table[][] Generate Voronoi diagram
---@field generateRandomPoints fun(width: number, height: number, count: number, seed?: number): table[] Generate random points
---@field generateMaze fun(width: number, height: number, options?: MazeOptions): table[][] Generate maze
---@field generateDungeon fun(width: number, height: number, options?: DungeonOptions): DungeonResult Generate BSP dungeon
algo = {}

---@class CellularAutomataOptions
---@field iterations? number Number of iterations (default 4)
---@field birthLimit? number Neighbors to become wall (default 4)
---@field deathLimit? number Neighbors to stay wall (default 3)
---@field width? number Grid width
---@field height? number Grid height

---@class CaveOptions
---@field fillProbability? number Initial fill probability (default 0.45)
---@field iterations? number CA iterations (default 4)
---@field birthLimit? number Birth threshold (default 4)
---@field deathLimit? number Death threshold (default 3)
---@field seed? number Random seed

---@class ErosionOptions
---@field iterations? number Number of droplets (default 50000)
---@field erosionRadius? number Erosion brush radius (default 3)
---@field inertia? number Droplet inertia (default 0.05)
---@field sedimentCapacity? number Max sediment capacity (default 4.0)
---@field minSlope? number Minimum slope (default 0.01)
---@field erosionSpeed? number Erosion rate (default 0.3)
---@field depositSpeed? number Deposit rate (default 0.3)
---@field evaporateSpeed? number Evaporation rate (default 0.01)
---@field gravity? number Gravity factor (default 4.0)
---@field seed? number Random seed
---@field maxDropletLifetime? number Max steps per droplet (default 30)

---@class ThermalErosionOptions
---@field iterations? number Number of iterations (default 50)
---@field talusAngle? number Max slope before erosion (default 0.5)
---@field erosionAmount? number Erosion transfer rate (default 0.5)

---@class SmoothOptions
---@field iterations? number Number of passes (default 1)
---@field kernelSize? number Kernel size (default 3)

---@class MazeOptions
---@field seed? number Random seed

---@class DungeonOptions
---@field minRoomSize? number Minimum room size (default 5)
---@field maxRoomSize? number Maximum room size (default 15)
---@field seed? number Random seed
---@field maxDepth? number BSP tree depth (default 4)

---@class DungeonResult
---@field grid table[][] The dungeon grid (1=wall, 0=floor)
---@field rooms Room[] Array of room definitions

---@class Room
---@field x number Room X position
---@field y number Room Y position
---@field width number Room width
---@field height number Room height

---@class Geo
---@field bresenhamLine fun(x1: number, y1: number, x2: number, y2: number): Point[] Points on a line
---@field bresenhamLine3d fun(x1: number, y1: number, z1: number, x2: number, y2: number, z2: number): Point3D[] 3D line points
---@field bezierCurve fun(points: Point[], steps?: number): Point[] Bezier curve through control points
---@field bezierCurve3d fun(points: Point3D[], steps?: number): Point3D[] 3D Bezier curve
---@field floodFill fun(grid: table[][], startX: number, startY: number, newValue: number, options?: FloodFillOptions): table[][] Flood fill algorithm
---@field getFloodFillPositions fun(grid: table[][], startX: number, startY: number, options?: FloodFillOptions): Point[] Get positions without modifying
---@field circle fun(centerX: number, centerY: number, radius: number, options?: ShapeOptions): Point[] Circle points
---@field ellipse fun(centerX: number, centerY: number, radiusX: number, radiusY: number, options?: ShapeOptions): Point[] Ellipse points
---@field rectangle fun(x1: number, y1: number, x2: number, y2: number, options?: ShapeOptions): Point[] Rectangle points
---@field polygon fun(vertices: Point[], options?: ShapeOptions): Point[] Polygon outline
---@field distance fun(x1: number, y1: number, x2: number, y2: number): number Euclidean distance
---@field distanceSq fun(x1: number, y1: number, x2: number, y2: number): number Squared distance (faster)
---@field distanceManhattan fun(x1: number, y1: number, x2: number, y2: number): number Manhattan distance
---@field distanceChebyshev fun(x1: number, y1: number, x2: number, y2: number): number Chebyshev distance
---@field pointInCircle fun(px: number, py: number, cx: number, cy: number, radius: number): boolean Point in circle test
---@field pointInRectangle fun(px: number, py: number, x1: number, y1: number, x2: number, y2: number): boolean Point in rectangle test
---@field pointInPolygon fun(px: number, py: number, vertices: Point[]): boolean Point in polygon test (ray casting)
---@field randomScatter fun(x1: number, y1: number, x2: number, y2: number, count: number, options?: ScatterOptions): Point[] Random scatter
---@field poissonDiskSampling fun(x1: number, y1: number, x2: number, y2: number, minDistance: number, options?: PoissonOptions): Point[] Blue noise distribution
geo = {}

---@class Point
---@field x number
---@field y number

---@class Point3D
---@field x number
---@field y number
---@field z number

---@class FloodFillOptions
---@field eightConnected? boolean Use 8-directional connectivity (default false)

---@class ShapeOptions
---@field filled? boolean Fill the shape (default false for outline)

---@class ScatterOptions
---@field seed? number Random seed
---@field minDistance? number Minimum distance between points (default 0)

---@class PoissonOptions
---@field seed? number Random seed
---@field maxAttempts? number Attempts per point (default 30)
