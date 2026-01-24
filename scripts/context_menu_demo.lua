-- @Title: Context Menu Demo
-- @Author: Michy
-- @Description: Demonstrates adding a custom context menu item using the new app.addContextMenu API.
-- @Version: 1.0.0
-- @Autorun: true

-- Function to handle the menu click
local function onMyMenuClick()
    if app.selection.isEmpty then
        app.alert("You clicked the menu, but nothing is selected!")
    else
        local count = app.selection.size
        app.alert("You clicked the menu! Selection size: " .. count)
    end
end

-- Register the menu item
-- This will appear in the right-click menu
app.addContextMenu("My Custom Tool", onMyMenuClick)

-- Register another one
app.addContextMenu("Another Tool", function()
    app.alert("Anonymous function callback works too!")
end)
