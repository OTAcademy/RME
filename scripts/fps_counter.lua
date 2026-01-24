-- @Title: FPS Counter
-- @Author: Michy
-- @Description: Displays real-time FPS (frames per second) counter as an overlay in the top-right corner. Shows current FPS, average FPS, and frame time. Note: FPS reflects actual canvas redraw rate.
-- @Version: 1.0.0
-- @Autorun: true

-- FPS tracking variables
local frameCount = 0
local lastTime = app.getTime()  -- Use high-precision timer (milliseconds since app start)
local currentFPS = 0
local fpsHistory = {}
local maxHistory = 60  -- Store last 60 FPS values for averaging

-- Configuration
local config = {
    updateInterval = 500,  -- Update FPS display every 500 milliseconds
    showBackground = true,
    showAverage = true,
    bgColor = { r = 20, g = 20, b = 20, a = 180 },
    textColor = { r = 0, g = 255, b = 100, a = 255 },
    warningColor = { r = 255, g = 200, b = 0, a = 255 },  -- Yellow for < 30 FPS
    criticalColor = { r = 255, g = 80, b = 80, a = 255 }, -- Red for < 15 FPS
    padding = 8,
    margin = 10,
    width = 85,
    height = 38
}

-- Get the appropriate color based on FPS value
local function getFPSColor(fps)
    if fps < 15 then
        return config.criticalColor
    elseif fps < 30 then
        return config.warningColor
    else
        return config.textColor
    end
end

-- Calculate average FPS from history
local function getAverageFPS()
    if #fpsHistory == 0 then
        return 0
    end
    local sum = 0
    for _, fps in ipairs(fpsHistory) do
        sum = sum + fps
    end
    return sum / #fpsHistory
end

-- Add FPS value to history
local function addToHistory(fps)
    table.insert(fpsHistory, fps)
    if #fpsHistory > maxHistory then
        table.remove(fpsHistory, 1)
    end
end

-- The ondraw callback receives a context with drawing functions
local function onDraw(ctx)
    -- Count this frame
    frameCount = frameCount + 1

    -- Calculate elapsed time using high-precision timer (milliseconds)
    local currentTime = app.getTime()
    local elapsed = currentTime - lastTime

    -- Update FPS calculation at configured interval
    if elapsed >= config.updateInterval then
        -- Convert elapsed from ms to seconds for FPS calculation
        currentFPS = frameCount / (elapsed / 1000)
        addToHistory(currentFPS)
        frameCount = 0
        lastTime = currentTime
    end

    -- Get screen dimensions from view context
    local screenWidth = ctx.view.screenWidth or 800
    local screenHeight = ctx.view.screenHeight or 600

    -- Position in top-right corner (screen space coordinates)
    local x = screenWidth - config.width - config.margin
    local y = config.margin

    -- Draw background panel if enabled
    if config.showBackground then
        ctx.rect({
            screen = true,
            x = x,
            y = y,
            w = config.width,
            h = config.showAverage and config.height + 14 or config.height,
            filled = true,
            color = config.bgColor
        })

        -- Draw border
        ctx.rect({
            screen = true,
            x = x,
            y = y,
            w = config.width,
            h = config.showAverage and config.height + 14 or config.height,
            filled = false,
            width = 1,
            color = { r = 60, g = 60, b = 60, a = 200 }
        })
    end

    -- Get color based on current FPS
    local fpsColor = getFPSColor(currentFPS)

    -- Draw FPS text
    local fpsText = string.format("FPS: %.1f", currentFPS)
    ctx.text({
        screen = true,
        x = x + config.padding,
        y = y + config.padding,
        text = fpsText,
        color = fpsColor
    })

    -- Draw average FPS if enabled
    if config.showAverage then
        local avgFPS = getAverageFPS()
        local avgColor = getFPSColor(avgFPS)
        local avgText = string.format("Avg: %.1f", avgFPS)
        ctx.text({
            screen = true,
            x = x + config.padding,
            y = y + config.padding + 16,
            text = avgText,
            color = { r = 180, g = 180, b = 180, a = 255 }
        })

        -- Draw frame time in ms
        local frameTimeMs = currentFPS > 0 and (1000 / currentFPS) or 0
        local frameTimeText = string.format("%.1fms", frameTimeMs)
        ctx.text({
            screen = true,
            x = x + config.padding,
            y = y + config.padding + 32,
            text = frameTimeText,
            color = { r = 150, g = 150, b = 150, a = 255 }
        })
    end
end

-- Register the overlay
app.mapView.addOverlay("fps_counter", {
    enabled = true,
    order = 1000,  -- High order to render on top
    ondraw = onDraw
})

-- Register in the View > Show menu for toggling
app.mapView.registerShow("FPS Counter", "fps_counter", {
    enabled = true,
    ontoggle = function(enabled)
        if enabled then
            -- Reset counters when re-enabled
            frameCount = 0
            lastTime = app.getTime()
            fpsHistory = {}
        end
    end
})

print("FPS Counter overlay loaded!")
