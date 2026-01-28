
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
		title = "RME Modern UI Demo",
		width = 650,
		height = 400,
		resizable = true,
		dockable = is_dockable,
		activeTab = "Modern Styles", -- Set default tab to the new one
		onclose = function()
			print("Demo dialog closed.")
		end
	}

	-- ----------------------------------------------------------------------------
	-- Modern Header Panel (Using new capabilities)
	-- ----------------------------------------------------------------------------
	dlg:panel({ bgcolor = Color.darkGray, padding = 10, expand = false, height = 30 })
		dlg:label({
			text = "RME SCRIPTING ENGINE",
			fgcolor = Color.white,
			font_size = 14,
			font_weight = "bold",
			align = "center"
		})
		dlg:label({
			text = "Demonstrating Advanced Styling & Flex Layout",
			fgcolor = Color.lighten(Color.gray, 30),
			font_size = 9,
			align = "center"
		})
	dlg:endpanel()

	-- ----------------------------------------------------------------------------
	-- Tab 1: Modern Styles (The new stuff!)
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Modern Styles" }

	dlg:panel({ bgcolor = "#F8FAFC", padding = 15, margin = 5, expand = true })
		dlg:label({ text = "Container Panels & Boxes", font_weight = "bold", font_size = 12 })

		dlg:box({ label = "Fixed Size & Alignment", margin = 5, bgcolor = Color.white, expand = false, align = "center" })
			dlg:label({ text = "This box has a custom background and is centered.", align = "center", margin = 10 })
			dlg:button({ text = "I'm a styled button", bgcolor = Color.blue, fgcolor = "white", width = 180, align = "center", rounded = true, hover = { bgcolor = Color.darken(Color.blue, 10) } })
		dlg:endbox()

		dlg:newrow()

		dlg:box({ label = "Color Palette Demo", orient = "horizontal", padding = 10, expand = true })
			dlg:panel({ bgcolor = Color.red, width = 60, height = 40, margin = 2, expand = false })
				dlg:label({ text = "Red", fgcolor = "white", align = "center", valign = "center" })
			dlg:endpanel()
			dlg:panel({ bgcolor = Color.green, width = 60, height = 40, margin = 2, expand = false })
				dlg:label({ text = "Green", fgcolor = "white", align = "center", valign = "center" })
			dlg:endpanel()
			dlg:panel({ bgcolor = Color.blue, width = 60, height = 40, margin = 2, expand = false })
				dlg:label({ text = "Blue", fgcolor = "white", align = "center", valign = "center" })
			dlg:endpanel()
			dlg:panel({ bgcolor = Color.orange, width = 60, height = 40, margin = 2, expand = false })
				dlg:label({ text = "Orange", fgcolor = "white", align = "center", valign = "center" })
			dlg:endpanel()
		dlg:endbox()

		dlg:separator()

		dlg:label({ text = "Interactive Theme Switcher", font_weight = "bold" })
		dlg:box({ orient = "horizontal", align = "center" })
			dlg:button({
				text = "Dark Mode",
				bgcolor = "#1A202C",
				fgcolor = "#EDF2F7",
				hover = { bgcolor = Color.darken(Color.black, 10) },
				onclick = function(d)
					d:modify({
						style_preview = { bgcolor = "#2D3748", fgcolor = "#F7FAFC" },
						style_lbl = { text = "Viewing: Dark Theme", fgcolor = Color.green }
					})
				end
			})
			dlg:button({
				text = "Light Mode",
				bgcolor = "#EDF2F7",
				fgcolor = "#1A202C",
				onclick = function(d)
					d:modify({
						style_preview = { bgcolor = Color.white, fgcolor = Color.black },
						style_lbl = { text = "Viewing: Light Theme", fgcolor = Color.blue }
					})
				end
			})
		dlg:endbox()

		dlg:panel({ id = "style_preview", bgcolor = Color.white, margin = 10, padding = 15, expand = true })
			dlg:label({ id = "style_lbl", text = "This panel can be dynamically updated using d:modify()", align = "center" })
		dlg:endpanel()
	dlg:endpanel()


	-- ----------------------------------------------------------------------------
	-- Tab 2: Basic Widgets (Updated with light styling)
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Basic Widgets" }
	dlg:box { orient = "vertical", label = "Classic Control Set", padding = 10 }
		dlg:label { text = "Standard inputs with optional alignment:", font_weight = "bold" }

		dlg:input { id = "t_input", label = "Text Entry:", text = "Edit me!", expand = true }
		dlg:number { id = "t_number", label = "Number Spinner:", value = 100, min = 0, max = 500, align = "left" }
		dlg:slider { id = "t_slider", label = "Slider:", value = 50, min = 0, max = 100, expand = true }

		dlg:box({ orient = "horizontal" })
			dlg:color { id = "t_color", label = "Color Picker:", color = { red = 100, green = 200, blue = 255 } }
			dlg:file { id = "t_file", label = "File Selection:", filename = "test.txt", save = false, expand = true }
		dlg:endbox()
	dlg:endbox()

	dlg:box { orient = "horizontal", label = "Toggles", expand = true }
		dlg:panel({ padding = 5, expand = true })
			dlg:check { id = "t_check_1", text = "Enable Feature A", selected = true }
			dlg:check { id = "t_check_2", text = "Enable Feature B", selected = false }
		dlg:endpanel()
		dlg:panel({ bgcolor = "#f0f0f0", padding = 5, expand = true })
			dlg:radio { id = "t_radio_1", text = "High Performance", selected = true }
			dlg:radio { id = "t_radio_2", text = "Power Saving", selected = false }
		dlg:endpanel()
	dlg:endbox()

	dlg:combobox { id = "t_combo", label = "Select Category:", options = { "Red", "Green", "Blue", "Alpha" }, option = "Green", margin = 10 }

	dlg:button { text = "Debug Current State", bgcolor = Color.darkGray, fgcolor = "white", align = "right", margin = 10, onclick = function(d)
		local data = d.data
		local info = string.format(
			"Input: %s\nNumber: %d\nSlider: %d\nColor: %s\nCheck A: %s\nRadio 1: %s",
			data.t_input, data.t_number, data.t_slider, colorToString(data.t_color),
			tostring(data.t_check_1), tostring(data.t_radio_1)
		)
		app.alert(info)
	end }

	-- ----------------------------------------------------------------------------
	-- Tab 3: Visuals & Lists
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Visuals & Lists" }

	dlg:wrap({ padding = 5 })
		dlg:box { orient = "vertical", label = "Engine Sprites", width = 150, expand = false }
			dlg:label { text = "Item ID 2160:", align = "center" }
			dlg:image { id = "img_item", itemid = 2160, width = 64, height = 64, align = "center", smooth = false }
			dlg:label { text = "Raw Sprite 100:", align = "center" }
			dlg:image { id = "img_sprite", spriteid = 100, width = 64, height = 64, align = "center" }
		dlg:endbox()

		dlg:box { orient = "vertical", label = "Map Viewport", expand = true }
			dlg:mapCanvas { id = "preview_canvas", expand = true, height = 200 }
			dlg:label { text = "Live preview of the map", align = "center", font_size = 8 }
		dlg:endbox()
	dlg:endwrap()

	dlg:box { orient = "horizontal", label = "Advanced View Widgets", expand = true }
		dlg:panel({ expand = true })
			dlg:label({ text = "Custom ListBox", font_weight = "bold" })
			dlg:list {
				id = "demo_list",
				height = 180,
				expand = true,
				items = {
					{ text = "Legendary Sword",   icon = 2160, tooltip = "Deals massive damage" },
					{ text = "Health Potion",    icon = 2152, tooltip = "Restores 100 HP" },
					{ text = "Mystery Key",      icon = 2148, tooltip = "What does it open?" }
				}
			}
		dlg:endpanel()

		dlg:panel({ expand = true })
			dlg:label({ text = "Item Grid", font_weight = "bold" })
			dlg:grid {
				id = "demo_grid",
				height = 180,
				cell_size = 48,
				item_size = 32,
				expand = true,
				items = {
					{ tooltip = "Gold",      image = Image.fromItemSprite(2148) },
					{ tooltip = "Platinum",   image = Image.fromItemSprite(2152) },
					{ tooltip = "Crystal",    image = Image.fromItemSprite(2160) },
					{ tooltip = "Raw 100", image = Image.fromSprite(100) },
					{ tooltip = "Raw 101", image = Image.fromSprite(101) }
				}
			}
		dlg:endpanel()
	dlg:endbox()

	-- ----------------------------------------------------------------------------
	-- Tab 4: Environment
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "Environment" }

	dlg:panel({ bgcolor = "#EDF2F7", padding = 15, margin = 10, expand = true })
		local map_info_text = "No map loaded."
		if app.map then
			map_info_text = string.format("Current Map: %s\nDimensions: %dx%d\nTotal Tiles: %d",
				app.map.name or "Untitled",
				app.map.width, app.map.height,
				app.map.tileCount or 0
			)
		end
		dlg:label { id = "lbl_map_info", text = map_info_text, font_size = 10 }
	dlg:endpanel()

	dlg:box({ orient = "horizontal", align = "center" })
		dlg:button { text = "Inspect Selection", width = 150, onclick = function()
			local sel = app.selection
			if not sel or sel.isEmpty then
				app.alert("Selection is empty.")
			else
				app.alert(string.format("Selected Tiles: %d", sel.size))
			end
		end }

		dlg:button { text = "Add Blueberry Bushes", bgcolor = "#2B6CB0", fgcolor = "white", width = 150, hover = { bgcolor = "#2C5282" }, onclick = function()
			if not app.map or app.selection.isEmpty then
				app.alert("Select map area first!")
				return
			end
			app.transaction("Add Bushes", function()
				for _, tile in ipairs(app.selection.tiles) do
					tile:addItem(2785, 1)
				end
			end)
		end }
	dlg:endbox()

	-- ----------------------------------------------------------------------------
	-- Tab 5: System
	-- ----------------------------------------------------------------------------
	dlg:tab { text = "System" }

	dlg:box({ orient = "vertical", padding = 10 })
		dlg:label { text = "Application Version", font_weight = "bold" }
		dlg:panel({ bgcolor = Color.lightGray, padding = 5 })
			dlg:label { text = app.version, align = "center" }
		dlg:endpanel()

		dlg:separator()

		local dock_btn_text = is_dockable and "Switch to Floating Window" or "Switch to Dockable Side-Panel"
		dlg:button { text = dock_btn_text, expand = true, onclick = function(d)
			d:close()
			createAndShowDialog(not is_dockable)
		end }

		dlg:separator()

		dlg:label { text = "Network Demo:", font_weight = "bold" }
		dlg:panel({ id = "http_box", bgcolor = "#FFF", height = 60, padding = 10, expand = true })
			dlg:label { text = "Result will appear here...", id = "lbl_http_res", align = "center", expand = true }
		dlg:endpanel()

		dlg:button { text = "Fetch Random Quote", bgcolor = Color.blue, fgcolor = "white", hover = { bgcolor = Color.darken(Color.blue, 10) }, expand = true, onclick = function(d)
			if not http then return end
			d:modify { lbl_http_res = { text = "Fetching..." }, http_box = { bgcolor = "#E2E8F0" } }
			local res = http.get("https://dummyjson.com/quotes/random")
			if res.ok and json then
				local data = json.decode(res.body)
				d:modify {
					lbl_http_res = { text = string.format('"%s"', data.quote or "?") },
					http_box = { bgcolor = Color.white }
				}
			else
				d:modify { lbl_http_res = { text = "Request failed" } }
			end
		end }
	dlg:endbox()

	dlg:endtabs()

	-- Footer
	dlg:separator()
	dlg:button { text = "CLOSE DEMO", align = "center", margin = 5, onclick = function(d) d:close() end }

	dlg:show { wait = false }
end

-- ============================================================================
-- Entry Point
-- ============================================================================

-- Start with defaults
createAndShowDialog(false)

print("Hello World demo executed.")
