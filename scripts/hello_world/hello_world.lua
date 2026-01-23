
-- Check if app table exists (sanity check)
if not app then
	print("Error: RME Lua API not found.")
	return
end

-- ============================================================================
-- Helper Functions
-- ============================================================================

-- Helper to format color for display
local function colorToString(c)
	if not c then return "nil" end
	return string.format("R:%d G:%d B:%d", c.red, c.green, c.blue)
end

-- Forward declaration
local createAndShowDialog

-- ============================================================================
-- Main Dialog Factory
-- ============================================================================

createAndShowDialog = function(is_dockable)
	local dlg = Dialog {
		title = "RME Lua API Capabilities Demo",
		width = 600,
		height = 500,
		resizable = true,
		dockable = is_dockable,
		activeTab = "Basic Widgets", -- Set default tab
		onclose = function()
			print("Demo dialog closed.")
		end
	}

	-- ----------------------------------------------------------------------------
	-- Tab 1: Basic Widgets
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Basic Widgets", oncontextmenu = function(d, info)
		return {
			{ text = "Tab Context Menu", onclick = function() app.alert("Clicked Tab Context") end }
		}
	end }
	dlg:box { orient = "vertical", label = "Input Fields" }
	dlg:label { text = "Standard inputs available in the API:" }
	dlg:newrow()

	dlg:input { id = "t_input", label = "Text Entry:", text = "Edit me!" }
	dlg:number { id = "t_number", label = "Number Spinner:", value = 100, min = 0, max = 500 }
	dlg:slider { id = "t_slider", label = "Slider:", value = 50, min = 0, max = 100 }
	dlg:color { id = "t_color", label = "Color Picker:", color = { red = 100, green = 200, blue = 255 } }
	dlg:file { id = "t_file", label = "File Picker:", filename = "test.txt", save = false }
	dlg:endbox()

	dlg:separator()

	dlg:box { orient = "horizontal", label = "Toggles & Choices" }
	dlg:box { orient = "vertical" }
	dlg:check { id = "t_check_1", text = "Checkbox Option A", selected = true }
	dlg:check { id = "t_check_2", text = "Checkbox Option B", selected = false }
	dlg:endbox()

	dlg:box { orient = "vertical" }
	dlg:radio { id = "t_radio_1", text = "Radio Mode 1", selected = true }
	dlg:radio { id = "t_radio_2", text = "Radio Mode 2", selected = false }
	dlg:endbox()

	dlg:combobox { id = "t_combo", label = "Combobox:", options = { "Red", "Green", "Blue", "Alpha" }, option = "Green" }
	dlg:endbox()

	dlg:separator()
	dlg:button { text = "Read Values", onclick = function(d)
		local data = d.data
		local info = string.format(
			"Input: %s\nNumber: %d\nSlider: %d\nColor: %s\nCheck A: %s\nRadio 1: %s",
			data.t_input, data.t_number, data.t_slider, colorToString(data.t_color),
			tostring(data.t_check_1), tostring(data.t_radio_1)
		)
		app.alert(info)
		d:modify { t_input = { label = "Updated Label:" } }
	end }

	-- ----------------------------------------------------------------------------
	-- Tab 2: Visuals & Lists
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Visuals & Lists" }

	dlg:wrap({})
	-- Image Widget demos
	dlg:box { orient = "vertical", label = "Images" }
	dlg:label { text = "Item Sprite (2160):" }
	dlg:image { id = "img_item", itemid = 2160, width = 32, height = 32, smooth = false }

	dlg:label { text = "Raw Sprite (100):" }
	dlg:image { id = "img_sprite", spriteid = 100, width = 32, height = 32 }
	dlg:endbox()

	-- Map Canvas
	dlg:box { orient = "vertical", label = "Map Preview" }
	dlg:mapCanvas { id = "preview_canvas", width = 150, height = 100 }
	dlg:endbox()
	dlg:endwrap()

	dlg:separator()

	-- LIST WIDGET
	dlg:box { orient = "horizontal", label = "List & Grid" }

	local last_click_time = 0

	dlg:list {
		id = "demo_list",
		width = 200,
		height = 200,
		show_text = true,
		items = {
			{ text = "Item 1 (Plain)",   tooltip = "Standard Item" },
			{ text = "Item 2 (Icon)",    icon = 2160,                  tooltip = "Item with Icon" },
			{ text = "Item 3 (Tooltip)", tooltip = "I have a tooltip!" },
			{ text = "Double Click Me",  icon = 2152,                  tooltip = "Double click test" }
		},
		onleftclick = function(d, info)
			-- Mimic favorites behavior: simple selection logic if needed
			print("List Left Click index: " .. tostring(info and info.index))
		end,
		ondoubleclick = function(d)
			-- WORKAROUND: Engine may trigger double click twice, using temporal debounce
			local now = os.clock()
			if now - last_click_time < 0.5 then
				return
			end
			last_click_time = now

			local sel = d.data.demo_list
			app.alert("List Double Click! Selection: " .. tostring(sel))
		end,
		oncontextmenu = function(d, info)
			-- Only show context menu if valid item clicked (favorites style)
			if info and info.index and info.index > 0 then
				return {
					{
						text = "List Action (Item " .. info.index .. ")",
						onclick = function()
							app.alert(
								"Clicked List Item " .. info.index)
						end
					},
					{ separator = true },
					{ text = "Delete", onclick = function() app.alert("Delete Action") end }
				}
			else
				-- Background context menu
				return {
					{ text = "List Background Action", onclick = function() app.alert("Clicked List Background") end }
				}
			end
		end
	}

	-- GRID WIDGET
	dlg:grid {
		id = "demo_grid",
		width = 200,
		height = 200,
		cell_size = 40,
		item_size = 32,
		show_text = true, -- Try forcing text if desired, though grid usually is icon-based
		items = {
			{ tooltip = "Coins",      image = Image.fromItemSprite(2148) },
			{ tooltip = "Platinum",   image = Image.fromItemSprite(2152) },
			{ tooltip = "Crystal",    image = Image.fromItemSprite(2160) },
			{ tooltip = "Sprite 100", image = Image.fromSprite(100) },
			{ tooltip = "Sprite 101", image = Image.fromSprite(101) }
		},
		onleftclick = function(d, info)
			print("Grid Left Click index: " .. tostring(info and info.index))
		end,
		oncontextmenu = function(d, info)
			-- Favorites style context menu
			if info and info.index and info.index > 0 then
				return {
					{ text = "Grid Item Action", onclick = function() app.alert("Grid Item " .. info.index) end }
				}
			end
			return {
				{ text = "Grid Background Action", onclick = function() app.alert("Grid BG") end }
			}
		end
	}
	dlg:endbox()

	-- ----------------------------------------------------------------------------
	-- Tab 3: Environment Engine
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Environment" }

	local map_info_text = "No map loaded."
	if app.map then
		map_info_text = string.format("Map: %s\nSize: %dx%d\nTiles: %d",
			app.map.name or "Untitled",
			app.map.width, app.map.height,
			app.map.tileCount or 0
		)
	end

	dlg:label { id = "lbl_map_info", text = map_info_text }

	dlg:separator()

	dlg:button { text = "Inspect Selection", onclick = function()
		local sel = app.selection
		if not sel or sel.isEmpty then
			app.alert("Selection is empty.\nSelect some tiles in the map editor first.")
		else
			local msg = string.format("Selected Tiles: %d\nBounds: (%d, %d, %d) to (%d, %d, %d)",
				sel.size,
				sel.minPosition.x, sel.minPosition.y, sel.minPosition.z,
				sel.maxPosition.x, sel.maxPosition.y, sel.maxPosition.z
			)
			app.alert(msg)
		end
	end }

	dlg:button { text = "Transaction Demo (Add Sparkles)", onclick = function()
		if not app.map then
			app.alert("No map loaded!")
			return
		end

		local sel = app.selection
		if sel.isEmpty then
			app.alert("Select area first to spawn sparkles (ID 2014)!")
			return
		end

		app.transaction("Demo Sparkles", function()
			for _, tile in ipairs(sel.tiles) do
				-- Add magic effect / sparkles
				tile:addItem(2785, 1)
			end
		end)
		app.alert("Added blueberry bushes to " .. sel.size .. " tiles.")
	end }

	dlg:separator()
	dlg:label { text = "Map Overlays (Scripting API)" }
	dlg:button { text = "Toggle 'Demo Overlay'", onclick = function()
		local overlay_id = "demo_overlay"
		app.mapView:addOverlay(overlay_id, {
			ondraw = function(ctx)
				ctx:rect { x = 10, y = 10, w = 200, h = 50, color = { r = 0, g = 0, b = 0, a = 100 }, screen = true, filled = true }
				ctx:text { x = 20, y = 25, text = "Demo Overlay Active", color = { r = 255, g = 255, b = 255 }, screen = true }
			end
		})
		app.alert("Overlay added. Move map to see updates if it was drawing world coords.")
	end }

	-- ----------------------------------------------------------------------------
	-- Tab 4: System & Network
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "System" }

	dlg:label { text = "Editor Version: " .. app.version }

	dlg:separator()

	-- DOCKABLE TOGGLE
	local dock_btn_text = is_dockable and "Reopen as Floating Window" or "Reopen as Dockable Window"
	dlg:button { text = dock_btn_text, onclick = function(d)
		d:close()
		createAndShowDialog(not is_dockable)
	end }

	dlg:separator()

	dlg:label { text = "HTTP Requests:" }
	dlg:label { text = "Result will appear here...", id = "lbl_http_res" }

	dlg:button { text = "Get Random Quote (HTTP JSON)", onclick = function(d)
		if not http then
			d:modify { lbl_http_res = { text = "Error: HTTP module not available." } }
			return
		end

		d:modify { lbl_http_res = { text = "Fetching..." } }

		local res = http.get("https://dummyjson.com/quotes/random")

		if res.ok then
			if json and json.decode then
				local status, data = pcall(json.decode, res.body)
				if status and data then
					local quote = data.quote or "No quote found"
					local author = data.author or "Unknown"
					local fmt = string.format('"%s" - %s', quote, author)
					d:modify { lbl_http_res = { text = fmt } }
				else
					d:modify { lbl_http_res = { text = "Invalid JSON: " .. string.sub(res.body, 1, 50) } }
				end
			else
				d:modify { lbl_http_res = { text = "Raw: " .. string.sub(res.body, 1, 50) .. "..." } }
			end
		else
			d:modify { lbl_http_res = { text = "HTTP Error: " .. (res.error or "Unknown") } }
		end
	end }

	dlg:separator()

	dlg:button { text = "Save Timestamp to Storage", onclick = function()
		local store = app.storage("hello_world_demo")
		local data = store:load() or {}
		data.last_run = os.time()
		store:save(data)
		app.alert("Saved timestamp: " .. data.last_run)
	end }

	dlg:endtabs()
	dlg:separator()

	dlg:button { text = "Close Demo", onclick = function(d) d:close() end }
	dlg:show { wait = false }
end

-- ============================================================================
-- Entry Point
-- ============================================================================

-- Start with defaults
createAndShowDialog(false)

print("Hello World demo executed.")
