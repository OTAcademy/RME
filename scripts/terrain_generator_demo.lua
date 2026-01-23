-- @Title: Terrain Generator Demo in-built
-- @Author: Michy
-- @Description: Demonstrates the procedural generation APIs (noise, algo, geo) with a rich UI for creating islands, caves, dungeons and more.
-- @Version: 1.0
-- @Shortcut: Ctrl+Shift+T

-- ============================================================================
-- Configuration & State
-- ============================================================================

local config = {
	-- General
	seed = os.time(),

	-- Island generator
	island = {
		waterTile = 4608,      -- Water
		sandTile = 4526,       -- Sand
		grassTile = 4526,      -- Grass
		forestTile = 4526,     -- Forest ground
		mountainTile = 919,    -- Mountain/rock
		frequency = 0.03,
		octaves = 4,
		waterLevel = -0.2,
		sandLevel = 0.0,
		forestLevel = 0.4,
		mountainLevel = 0.7,
		addTrees = true,
		treeDensity = 0.15,
	},

	-- Cave generator
	cave = {
		wallTile = 919,        -- Rock wall
		floorTile = 4526,      -- Cave floor
		fillProbability = 0.45,
		iterations = 5,
		birthLimit = 4,
		deathLimit = 3,
	},

	-- Dungeon generator
	dungeon = {
		wallTile = 919,
		floorTile = 4526,
		minRoomSize = 4,
		maxRoomSize = 10,
		maxDepth = 4,
	},

	-- River/Path
	river = {
		waterTile = 4608,
		pathWidth = 2,
		curveStrength = 30,
	},
}

-- Store for persistence
local store = app.storage and app.storage("terrain_generator_demo")
local savedConfig = store and store:load()
if savedConfig then
	-- Merge saved config (shallow)
	for k, v in pairs(savedConfig) do
		if type(v) == "table" and config[k] then
			for k2, v2 in pairs(v) do
				config[k][k2] = v2
			end
		else
			config[k] = v
		end
	end
end

-- ============================================================================
-- Helper Functions
-- ============================================================================

local function saveConfig()
	if store then
		store:save(config)
	end
end

local function getSelection()
	local sel = app.selection
	if not sel or sel.isEmpty then
		return nil
	end
	return sel.bounds
end

local function requireSelection()
	local bounds = getSelection()
	if not bounds then
		app.alert("Please select an area on the map first!")
		return nil
	end
	return bounds
end

-- Apply ground tile with auto-borderize
local function setGroundTile(x, y, z, tileId)
	local tile = app.map:getOrCreateTile(x, y, z)
	if tile then
		tile:addItem(tileId)
	end
end

-- Borderize an area after placing grounds
local function borderizeArea(x1, y1, x2, y2, z)
	for y = y1 - 1, y2 + 1 do
		for x = x1 - 1, x2 + 1 do
			local tile = app.map:getTile(x, y, z)
			if tile then
				tile:borderize()
			end
		end
	end
end

-- ============================================================================
-- Island Generator
-- ============================================================================

local function generateIsland(bounds, cfg)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1
	local centerX = width / 2
	local centerY = height / 2
	local maxRadius = math.min(centerX, centerY)

	local seed = cfg.seed or os.time()
	local treesPlaced = 0

	app.transaction("Generate Island", function()
		for y = minY, maxY do
			for x = minX, maxX do
				local localX = x - minX
				local localY = y - minY

				-- Get noise value
				local n = noise.fbm(x, y, seed, {
					frequency = cfg.frequency,
					octaves = cfg.octaves,
					noiseType = "simplex"
				})

				-- Apply radial falloff (island shape)
				local dx = localX - centerX
				local dy = localY - centerY
				local distFromCenter = math.sqrt(dx * dx + dy * dy) / maxRadius
				local falloff = 1 - distFromCenter * distFromCenter
				n = n * falloff

				-- Determine terrain type
				local tileId
				local canPlaceTree = false

				if n < cfg.waterLevel then
					tileId = cfg.waterTile
				elseif n < cfg.sandLevel then
					tileId = cfg.sandTile
				elseif n < cfg.forestLevel then
					tileId = cfg.grassTile
					canPlaceTree = cfg.addTrees
				elseif n < cfg.mountainLevel then
					tileId = cfg.forestTile
					canPlaceTree = cfg.addTrees
				else
					tileId = cfg.mountainTile
				end

				setGroundTile(x, y, z, tileId)

				-- Place trees
				if canPlaceTree and math.random() < cfg.treeDensity then
					local tile = app.map:getTile(x, y, z)
					if tile then
						-- Use doodad brush for trees if available
						local treeBrush = Brushes.get("palm trees") or Brushes.get("trees") or Brushes.get("green trees")
						if treeBrush then
							tile:applyBrush(treeBrush.name, false)
							treesPlaced = treesPlaced + 1
						end
					end
				end
			end

			-- Yield periodically to keep UI responsive
			if y % 10 == 0 then
				app.yield()
			end
		end

		-- Borderize
		borderizeArea(minX, minY, maxX, maxY, z)
	end)

	return { treesPlaced = treesPlaced }
end

-- ============================================================================
-- Cave Generator (Cellular Automata)
-- ============================================================================

local function generateCave(bounds, cfg)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1

	-- Generate cave map using algo API
	local caveMap = algo.generateCave(width, height, {
		fillProbability = cfg.fillProbability,
		iterations = cfg.iterations,
		birthLimit = cfg.birthLimit,
		deathLimit = cfg.deathLimit,
		seed = cfg.seed or os.time(),
	})

	app.transaction("Generate Cave", function()
		for y = 1, height do
			for x = 1, width do
				local mapX = minX + x - 1
				local mapY = minY + y - 1

				local isWall = caveMap[y] and caveMap[y][x] == 1
				local tileId = isWall and cfg.wallTile or cfg.floorTile

				setGroundTile(mapX, mapY, z, tileId)
			end

			if y % 10 == 0 then
				app.yield()
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)
end

-- ============================================================================
-- Dungeon Generator (BSP)
-- ============================================================================

local function generateDungeon(bounds, cfg)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1

	-- Generate dungeon using algo API
	local result = algo.generateDungeon(width, height, {
		minRoomSize = cfg.minRoomSize,
		maxRoomSize = cfg.maxRoomSize,
		maxDepth = cfg.maxDepth,
		seed = cfg.seed or os.time(),
	})

	local dungeonMap = result.grid
	local rooms = result.rooms

	app.transaction("Generate Dungeon", function()
		for y = 1, height do
			for x = 1, width do
				local mapX = minX + x - 1
				local mapY = minY + y - 1

				local isWall = dungeonMap[y] and dungeonMap[y][x] == 1
				local tileId = isWall and cfg.wallTile or cfg.floorTile

				setGroundTile(mapX, mapY, z, tileId)
			end

			if y % 10 == 0 then
				app.yield()
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)

	return { roomCount = #rooms }
end

-- ============================================================================
-- River/Path Generator (Bezier + Bresenham)
-- ============================================================================

local function generateRiver(bounds, cfg)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1

	-- Create control points for river curve
	local seed = cfg.seed or os.time()
	math.randomseed(seed)

	local controlPoints = {
		{ x = minX, y = minY + math.random(0, height - 1) },
		{ x = minX + width * 0.25 + math.random(-cfg.curveStrength, cfg.curveStrength), y = minY + math.random(0, height - 1) },
		{ x = minX + width * 0.75 + math.random(-cfg.curveStrength, cfg.curveStrength), y = minY + math.random(0, height - 1) },
		{ x = maxX, y = minY + math.random(0, height - 1) },
	}

	-- Get bezier curve points
	local riverPath = geo.bezierCurve(controlPoints, width * 2)

	app.transaction("Generate River", function()
		-- Draw river with width
		for _, point in ipairs(riverPath) do
			local cx = math.floor(point.x)
			local cy = math.floor(point.y)

			-- Draw circle at each point for river width
			local circlePoints = geo.circle(cx, cy, cfg.pathWidth, { filled = true })
			for _, cp in ipairs(circlePoints) do
				if cp.x >= minX and cp.x <= maxX and cp.y >= minY and cp.y <= maxY then
					setGroundTile(cp.x, cp.y, z, cfg.waterTile)
				end
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)

	return { pointCount = #riverPath }
end

-- ============================================================================
-- Noise Preview
-- ============================================================================

local function generateNoisePreview(bounds, noiseType, seed, frequency)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y

	-- Define gradient from water to mountain
	local gradientTiles = {
		4608, -- Deep water (n < -0.6)
		4608, -- Water (n < -0.2)
		4526, -- Sand (n < 0.0)
		4526, -- Grass (n < 0.3)
		4526, -- Forest (n < 0.6)
		919,  -- Mountain (n >= 0.6)
	}

	app.transaction("Noise Preview", function()
		for y = minY, maxY do
			for x = minX, maxX do
				local n

				if noiseType == "perlin" then
					n = noise.perlin(x, y, seed, frequency)
				elseif noiseType == "simplex" then
					n = noise.simplex(x, y, seed, frequency)
				elseif noiseType == "cellular" then
					n = noise.cellular(x, y, seed, frequency)
				elseif noiseType == "fbm" then
					n = noise.fbm(x, y, seed, { frequency = frequency, octaves = 4 })
				elseif noiseType == "ridged" then
					n = noise.ridged(x, y, seed, { frequency = frequency })
				else
					n = noise.simplex(x, y, seed, frequency)
				end

				-- Map noise to gradient
				local idx = math.floor(noise.normalize(n, 1, #gradientTiles))
				idx = math.max(1, math.min(idx, #gradientTiles))

				setGroundTile(x, y, z, gradientTiles[idx])
			end

			if y % 10 == 0 then
				app.yield()
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)
end

-- ============================================================================
-- Voronoi Regions
-- ============================================================================

local function generateVoronoi(bounds, numRegions)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1

	-- Generate seed points
	local points = algo.generateRandomPoints(width, height, numRegions, config.seed)

	-- Generate Voronoi diagram
	local voronoiMap = algo.voronoi(width, height, points)

	-- Define tiles for each region (cycle through)
	local regionTiles = { 4526, 4608, 919, 4526, 4608, 919 }

	app.transaction("Generate Voronoi", function()
		for y = 1, height do
			for x = 1, width do
				local mapX = minX + x - 1
				local mapY = minY + y - 1

				local region = voronoiMap[y] and voronoiMap[y][x] or 1
				local tileId = regionTiles[((region - 1) % #regionTiles) + 1]

				setGroundTile(mapX, mapY, z, tileId)
			end

			if y % 10 == 0 then
				app.yield()
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)

	return { regionCount = numRegions }
end

-- ============================================================================
-- Maze Generator
-- ============================================================================

local function generateMaze(bounds, wallTile, floorTile)
	local minX, minY, z = bounds.min.x, bounds.min.y, bounds.min.z
	local maxX, maxY = bounds.max.x, bounds.max.y
	local width = maxX - minX + 1
	local height = maxY - minY + 1

	local mazeMap = algo.generateMaze(width, height, { seed = config.seed })
	local mazeHeight = #mazeMap
	local mazeWidth = mazeMap[1] and #mazeMap[1] or 0

	app.transaction("Generate Maze", function()
		for y = 1, mazeHeight do
			for x = 1, mazeWidth do
				local mapX = minX + x - 1
				local mapY = minY + y - 1

				if mapX <= maxX and mapY <= maxY then
					local isWall = mazeMap[y] and mazeMap[y][x] == 1
					local tileId = isWall and wallTile or floorTile
					setGroundTile(mapX, mapY, z, tileId)
				end
			end

			if y % 10 == 0 then
				app.yield()
			end
		end

		borderizeArea(minX, minY, maxX, maxY, z)
	end)
end

-- ============================================================================
-- Dialog UI
-- ============================================================================

local function createAndShowDialog()
	local dlg = Dialog {
		title = "Terrain Generator",
		width = 500,
		height = 550,
		resizable = true,
		dockable = true,
		onclose = function()
			saveConfig()
		end
	}

	-- ========================================
	-- Tab 1: Island Generator
	-- ========================================
	dlg:tab { text = "Island" }

	dlg:box { orient = "vertical", label = "Noise Settings" }
	dlg:number { id = "island_seed", label = "Seed:", value = config.seed, min = 0, max = 999999999 }
	dlg:slider { id = "island_frequency", label = "Frequency (x100):", value = config.island.frequency * 100, min = 1, max = 20 }
	dlg:slider { id = "island_octaves", label = "Octaves:", value = config.island.octaves, min = 1, max = 8 }
	dlg:endbox()

	dlg:box { orient = "vertical", label = "Terrain Levels" }
	dlg:slider { id = "island_water", label = "Water Level:", value = (config.island.waterLevel + 1) * 50, min = 0, max = 100 }
	dlg:slider { id = "island_sand", label = "Sand Level:", value = (config.island.sandLevel + 1) * 50, min = 0, max = 100 }
	dlg:slider { id = "island_forest", label = "Forest Level:", value = (config.island.forestLevel + 1) * 50, min = 0, max = 100 }
	dlg:slider { id = "island_mountain", label = "Mountain Level:", value = (config.island.mountainLevel + 1) * 50, min = 0, max = 100 }
	dlg:endbox()

	dlg:box { orient = "horizontal", label = "Tile IDs" }
	dlg:item { id = "island_water_tile", label = "Water:", itemid = config.island.waterTile }
	dlg:item { id = "island_grass_tile", label = "Grass:", itemid = config.island.grassTile }
	dlg:item { id = "island_mountain_tile", label = "Mountain:", itemid = config.island.mountainTile }
	dlg:endbox()

	dlg:check { id = "island_trees", text = "Add Trees", selected = config.island.addTrees }
	dlg:slider { id = "island_tree_density", label = "Tree Density (%):", value = config.island.treeDensity * 100, min = 0, max = 50 }

	dlg:button { text = "Generate Island", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		-- Update config from UI
		config.seed = d.data.island_seed
		config.island.frequency = d.data.island_frequency / 100
		config.island.octaves = d.data.island_octaves
		config.island.waterLevel = d.data.island_water / 50 - 1
		config.island.sandLevel = d.data.island_sand / 50 - 1
		config.island.forestLevel = d.data.island_forest / 50 - 1
		config.island.mountainLevel = d.data.island_mountain / 50 - 1
		config.island.waterTile = d.data.island_water_tile
		config.island.grassTile = d.data.island_grass_tile
		config.island.mountainTile = d.data.island_mountain_tile
		config.island.addTrees = d.data.island_trees
		config.island.treeDensity = d.data.island_tree_density / 100

		local result = generateIsland(bounds, config.island)
		app.alert("Island generated! Trees placed: " .. result.treesPlaced)
		app.refresh()
	end }

	-- ========================================
	-- Tab 2: Cave Generator
	-- ========================================
	dlg:tab { text = "Cave" }

	dlg:box { orient = "vertical", label = "Cellular Automata Settings" }
	dlg:number { id = "cave_seed", label = "Seed:", value = config.seed, min = 0, max = 999999999 }
	dlg:slider { id = "cave_fill", label = "Fill Probability (%):", value = config.cave.fillProbability * 100, min = 30, max = 60 }
	dlg:slider { id = "cave_iterations", label = "Iterations:", value = config.cave.iterations, min = 1, max = 10 }
	dlg:slider { id = "cave_birth", label = "Birth Limit:", value = config.cave.birthLimit, min = 2, max = 6 }
	dlg:slider { id = "cave_death", label = "Death Limit:", value = config.cave.deathLimit, min = 2, max = 6 }
	dlg:endbox()

	dlg:box { orient = "horizontal", label = "Tile IDs" }
	dlg:item { id = "cave_wall_tile", label = "Wall:", itemid = config.cave.wallTile }
	dlg:item { id = "cave_floor_tile", label = "Floor:", itemid = config.cave.floorTile }
	dlg:endbox()

	dlg:button { text = "Generate Cave", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		config.seed = d.data.cave_seed
		config.cave.fillProbability = d.data.cave_fill / 100
		config.cave.iterations = d.data.cave_iterations
		config.cave.birthLimit = d.data.cave_birth
		config.cave.deathLimit = d.data.cave_death
		config.cave.wallTile = d.data.cave_wall_tile
		config.cave.floorTile = d.data.cave_floor_tile

		generateCave(bounds, config.cave)
		app.alert("Cave generated!")
		app.refresh()
	end }

	-- ========================================
	-- Tab 3: Dungeon Generator
	-- ========================================
	dlg:tab { text = "Dungeon" }

	dlg:box { orient = "vertical", label = "BSP Settings" }
	dlg:number { id = "dungeon_seed", label = "Seed:", value = config.seed, min = 0, max = 999999999 }
	dlg:slider { id = "dungeon_min_room", label = "Min Room Size:", value = config.dungeon.minRoomSize, min = 3, max = 10 }
	dlg:slider { id = "dungeon_max_room", label = "Max Room Size:", value = config.dungeon.maxRoomSize, min = 5, max = 20 }
	dlg:slider { id = "dungeon_depth", label = "BSP Depth:", value = config.dungeon.maxDepth, min = 2, max = 6 }
	dlg:endbox()

	dlg:box { orient = "horizontal", label = "Tile IDs" }
	dlg:item { id = "dungeon_wall_tile", label = "Wall:", itemid = config.dungeon.wallTile }
	dlg:item { id = "dungeon_floor_tile", label = "Floor:", itemid = config.dungeon.floorTile }
	dlg:endbox()

	dlg:button { text = "Generate Dungeon", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		config.seed = d.data.dungeon_seed
		config.dungeon.minRoomSize = d.data.dungeon_min_room
		config.dungeon.maxRoomSize = d.data.dungeon_max_room
		config.dungeon.maxDepth = d.data.dungeon_depth
		config.dungeon.wallTile = d.data.dungeon_wall_tile
		config.dungeon.floorTile = d.data.dungeon_floor_tile

		local result = generateDungeon(bounds, config.dungeon)
		app.alert("Dungeon generated! Rooms: " .. result.roomCount)
		app.refresh()
	end }

	-- ========================================
	-- Tab 4: More Tools
	-- ========================================
	dlg:tab { text = "More" }

	dlg:box { orient = "vertical", label = "River Generator" }
	dlg:item { id = "river_tile", label = "Water Tile:", itemid = config.river.waterTile }
	dlg:slider { id = "river_width", label = "Path Width:", value = config.river.pathWidth, min = 1, max = 5 }
	dlg:slider { id = "river_curve", label = "Curve Strength:", value = config.river.curveStrength, min = 0, max = 50 }
	dlg:button { text = "Generate River", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		config.river.waterTile = d.data.river_tile
		config.river.pathWidth = d.data.river_width
		config.river.curveStrength = d.data.river_curve

		generateRiver(bounds, config.river)
		app.alert("River generated!")
		app.refresh()
	end }
	dlg:endbox()

	dlg:separator()

	dlg:box { orient = "vertical", label = "Voronoi Regions" }
	dlg:slider { id = "voronoi_regions", label = "Num Regions:", value = 10, min = 3, max = 30 }
	dlg:button { text = "Generate Voronoi", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		local result = generateVoronoi(bounds, d.data.voronoi_regions)
		app.alert("Voronoi generated! Regions: " .. result.regionCount)
		app.refresh()
	end }
	dlg:endbox()

	dlg:separator()

	dlg:box { orient = "vertical", label = "Maze" }
	dlg:item { id = "maze_wall", label = "Wall:", itemid = 919 }
	dlg:item { id = "maze_floor", label = "Floor:", itemid = 4526 }
	dlg:button { text = "Generate Maze", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		generateMaze(bounds, d.data.maze_wall, d.data.maze_floor)
		app.alert("Maze generated!")
		app.refresh()
	end }
	dlg:endbox()

	-- ========================================
	-- Tab 5: Noise Preview
	-- ========================================
	dlg:tab { text = "Noise Test" }

	dlg:box { orient = "vertical", label = "Noise Preview" }
	dlg:label { text = "Visualize different noise types on the selected area." }
	dlg:number { id = "preview_seed", label = "Seed:", value = config.seed, min = 0, max = 999999999 }
	dlg:slider { id = "preview_freq", label = "Frequency (x100):", value = 3, min = 1, max = 20 }
	dlg:combobox { id = "preview_type", label = "Noise Type:", options = { "perlin", "simplex", "cellular", "fbm", "ridged" }, option = "simplex" }
	dlg:endbox()

	dlg:button { text = "Preview Noise", onclick = function(d)
		local bounds = requireSelection()
		if not bounds then return end

		generateNoisePreview(bounds, d.data.preview_type, d.data.preview_seed, d.data.preview_freq / 100)
		app.refresh()
	end }

	dlg:separator()

	dlg:label { text = "Tip: Use the noise, algo, and geo APIs directly in your scripts!" }
	dlg:label { text = "Example: local n = noise.simplex(x, y, seed, 0.05)" }

	dlg:endtabs()

	dlg:separator()

	dlg:box { orient = "horizontal" }
	dlg:button { text = "Randomize Seed", onclick = function(d)
		local newSeed = os.time() + math.random(0, 10000)
		d:modify {
			island_seed = { value = newSeed },
			cave_seed = { value = newSeed },
			dungeon_seed = { value = newSeed },
			preview_seed = { value = newSeed },
		}
		config.seed = newSeed
	end }
	dlg:button { text = "Close", onclick = function(d)
		d:close()
	end }
	dlg:endbox()

	dlg:show { wait = false }
end

-- ============================================================================
-- Entry Point
-- ============================================================================

if not app then
	app.alert("Error: RME Lua API not found.")
	return
end

if not noise then
	app.alert("Error: The 'noise' API is not available.\nPlease update RME to the latest version.")
	return
end

createAndShowDialog()
